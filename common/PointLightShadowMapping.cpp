#include"PointLightShadowMapping.h"
#include"Renderer.h"
#include"ShaderProgamMgr.h"
#include"FrameBuffer.h"
#include"Texture.h"
#include"Util.h"
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/transform.hpp>


PointLightShadowMapping::PointLightShadowMapping(Renderer* renderer, const glm::vec2& shadowMapResolution)
: m_renderer(renderer)
, m_shadowMapResolution(shadowMapResolution)
, m_rendererViewPort()
, m_FBO(nullptr)
, m_cubeShadowMap(nullptr){
	
}

PointLightShadowMapping::~PointLightShadowMapping() {
	cleanUp();
}

bool PointLightShadowMapping::initialize() {
	m_FBO.reset(new FrameBuffer());
	m_cubeShadowMap.reset(new Texture);

	m_cubeShadowMap->bind(Texture::Unit::Defualt, Texture::Target::Texture_CubeMap);
	if (!m_cubeShadowMap->loadCubeMapFromMemory(Texture::Format::Depth32,
		Texture::Format::Depth,
		Texture::FormatDataType::Float,
		m_shadowMapResolution.x,
		m_shadowMapResolution.y)) {
		cleanUp();
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return false;
	}
	m_cubeShadowMap->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
	m_cubeShadowMap->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
	m_cubeShadowMap->setWrapMode(Texture::WrapType::S, Texture::WrapMode::Clamp_To_Border);
	m_cubeShadowMap->setWrapMode(Texture::WrapType::T, Texture::WrapMode::Clamp_To_Border);
	m_cubeShadowMap->setWrapMode(Texture::WrapType::R, Texture::WrapMode::Clamp_To_Border);
	m_cubeShadowMap->setBorderColor({ 1.f, 1.f, 1.f, 1.f });

	m_FBO->bind();
	if (!m_FBO->addTextureAttachment(m_cubeShadowMap->getHandler(), FrameBuffer::AttachmentPoint::Depth)) {
		cleanUp();
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return false;
	}
	m_FBO->setDrawBufferLocation({});
	m_FBO->setReadBufferLocation(-1);
	FrameBuffer::Status status = m_FBO->checkStatus();
	m_FBO->unbind();
	m_cubeShadowMap->unbind();

	if (status != FrameBuffer::Status::Ok) {
		cleanUp();
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return false;
	}

	return true;
}

void PointLightShadowMapping::cleanUp() {
	m_FBO.release();
	m_cubeShadowMap.release();
}

void PointLightShadowMapping::beginShadowPhase(const Light_t& light, const Camera_t& camera) {
	auto shader = ShaderProgramManager::getInstance()->getProgram("PointLightShadowPass");
	if (shader.expired())
		shader = ShaderProgramManager::getInstance()->addProgram("PointLightShadowPass.shader");
	ASSERT(!shader.expired());

	std::shared_ptr<ShaderProgram> strongShader = shader.lock();
	strongShader->bind();
	m_FBO->bind();
	
	m_rendererViewPort = m_renderer->getViewport();
	m_renderer->clearScreen(ClearFlags::Depth);
	m_renderer->setViewPort(Viewport_t(0.f, 0.f, m_shadowMapResolution.x, m_shadowMapResolution.y));
	m_renderer->setCullFaceMode(CullFaceMode::Front);

	if (strongShader->hasUniform("u_lightVP[0]")) {
		auto transforms = calclightLightCameraMatrixs(light);
		strongShader->setUniformMat4v("u_lightVP[0]", glm::value_ptr(transforms.front()), transforms.size());
	}

	if (strongShader->hasUniform("u_near"))
		strongShader->setUniform1("u_near", 1.f);
	
	if (strongShader->hasUniform("u_far"))
		strongShader->setUniform1("u_far", light.range);

	m_renderer->pullingRenderTask(shader);

}

void PointLightShadowMapping::endShadowPhase(const Light_t& light) {
	m_FBO->unbind();
	FrameBuffer::bindDefault();
	m_renderer->setViewPort(m_rendererViewPort);
	m_renderer->setCullFaceMode(CullFaceMode::Back);
}

void PointLightShadowMapping::beginLighttingPhase(const Light_t& light, ShaderProgram* shader) {		
	if (shader->hasSubroutineUniform(Shader::Type::FragmentShader, "u_shadowAtten")) {
		if (light.shadowType == ShadowType::NoShadow) {
			shader->setSubroutineUniforms(Shader::Type::FragmentShader, { {"u_shadowAtten", "noShadow"} });
		}
		else if (light.shadowType == ShadowType::HardShadow) {
			shader->setSubroutineUniforms(Shader::Type::FragmentShader, { {"u_shadowAtten", "hardShadow"} });
		}
		else if (light.shadowType == ShadowType::SoftShadow) {
			shader->setSubroutineUniforms(Shader::Type::FragmentShader, { {"u_shadowAtten", "softShadow"} });
		}
	}

	if (shader->hasUniform("u_shadowMap") && light.isCastShadow()) {
		m_cubeShadowMap->bind(Texture::Unit::ShadowMap, Texture::Target::Texture_CubeMap);
		shader->setUniform1("u_shadowMap", int(Texture::Unit::ShadowMap));

		if (shader->hasUniform("u_shadowStrength"))
			shader->setUniform1("u_shadowStrength", light.shadowStrength);

		if (shader->hasUniform("u_shadowBias"))
			shader->setUniform1("u_shadowBias", light.shadowBias);
	}
}

void PointLightShadowMapping::endLighttingPhase(const Light_t& light, ShaderProgram* shader) {
	if (light.isCastShadow())
		m_cubeShadowMap->unbind();

}

void PointLightShadowMapping::onShadowMapResolutionChange(float w, float h) {
	m_shadowMapResolution = { w, h };
	bool ok = initialize();
#ifdef _DEBUG
	ASSERT(ok);
#endif // _DEBUG
}


std::vector<glm::mat4> PointLightShadowMapping::calclightLightCameraMatrixs(const Light_t& l) {
	std::vector<glm::mat4> transforms;
	glm::mat4 P = glm::perspective(glm::radians(90.f), m_shadowMapResolution.x / m_shadowMapResolution.y, 1.f, l.range);
	transforms.push_back(P * glm::lookAt(l.position, l.position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0))); // +x
	transforms.push_back(P * glm::lookAt(l.position, l.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0))); // -x
	transforms.push_back(P * glm::lookAt(l.position, l.position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0))); // +y
	transforms.push_back(P * glm::lookAt(l.position, l.position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0))); // -y
	transforms.push_back(P * glm::lookAt(l.position, l.position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0))); // +z
	transforms.push_back(P * glm::lookAt(l.position, l.position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0))); // -z

	return transforms;
}