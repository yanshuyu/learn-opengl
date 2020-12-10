#include"PointLightShadowMapping.h"
#include"Renderer.h"
#include"ShaderProgamMgr.h"
#include"Texture.h"
#include"Util.h"
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/transform.hpp>


PointLightShadowMapping::PointLightShadowMapping(IRenderTechnique* rt, const glm::vec2& shadowMapResolution)
: IShadowMapping(rt)
, m_shadowMapResolution(shadowMapResolution)
, m_shadowViewport(0.f, 0.f, shadowMapResolution.x, shadowMapResolution.y)
, m_shadowTarget(shadowMapResolution)
, m_shader(nullptr) {
	
}

PointLightShadowMapping::~PointLightShadowMapping() {
	cleanUp();
}

bool PointLightShadowMapping::initialize() {
	if (!m_shadowTarget.attachTextureCube(Texture::Format::Depth24, RenderTarget::Slot::Depth)) {
		cleanUp();
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return false;
	}

	if (!m_shadowTarget.isValid()) {
		cleanUp();
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return false;
	}

	Texture* shadowMap = m_shadowTarget.getAttachedTexture(RenderTarget::Slot::Depth);
#ifdef _DEBUG
	ASSERT(shadowMap);
#endif // _DEBUG
	if (shadowMap) {
		shadowMap->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
		shadowMap->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
		shadowMap->setWrapMode(Texture::WrapType::S, Texture::WrapMode::Clamp_To_Border);
		shadowMap->setWrapMode(Texture::WrapType::T, Texture::WrapMode::Clamp_To_Border);
		shadowMap->setWrapMode(Texture::WrapType::R, Texture::WrapMode::Clamp_To_Border);
		shadowMap->setBorderColor({ 1.f, 1.f, 1.f, 1.f });
	}

	return true;
}

void PointLightShadowMapping::cleanUp() {
	m_shadowTarget.detachAllTexture();
}

void PointLightShadowMapping::beginShadowPhase(const Scene_t& scene, const Light_t& light) {
	auto shader = ShaderProgramManager::getInstance()->getProgram("PointLightShadowPass");
	if (shader.expired())
		shader = ShaderProgramManager::getInstance()->addProgram("PointLightShadowPass.shader");
	ASSERT(!shader.expired());

	m_shader = shader.lock();
	auto renderer = m_renderTech->getRenderer();
	renderer->pushShaderProgram(m_shader.get());
	renderer->pushRenderTarget(&m_shadowTarget);
	renderer->pushViewport(&m_shadowViewport);
	renderer->clearScreen(ClearFlags::Depth);

	if (m_shader->hasUniform("u_lightVP[0]")) {
		auto transforms = calclightLightCameraMatrixs(light);
		m_shader->setUniformMat4v("u_lightVP[0]", glm::value_ptr(transforms.front()), transforms.size());
	}

	if (m_shader->hasUniform("u_near"))
		m_shader->setUniform1("u_near", 1.f);
	
	if (m_shader->hasUniform("u_far"))
		m_shader->setUniform1("u_far", light.range);

	for (size_t i = 0; i < scene.numOpaqueItems; i++) {
		m_renderTech->render(scene.opaqueItems[i]);
	}
}

void PointLightShadowMapping::endShadowPhase() {
	auto renderer = m_renderTech->getRenderer();
	renderer->popRenderTarget();
	renderer->popViewport();
	renderer->popShadrProgram();
	m_shader = nullptr;
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
		Texture* shadowMap = m_shadowTarget.getAttachedTexture(RenderTarget::Slot::Depth);
		if (shadowMap) {
			shadowMap->bind(Texture::Unit::ShadowMap, Texture::Target::Texture_CubeMap);
			shader->setUniform1("u_shadowMap", int(Texture::Unit::ShadowMap));
		}

		if (shader->hasUniform("u_shadowStrength"))
			shader->setUniform1("u_shadowStrength", light.shadowStrength);

		if (shader->hasUniform("u_shadowBias"))
			shader->setUniform1("u_shadowBias", light.shadowBias);
	}
}

void PointLightShadowMapping::endLighttingPhase(const Light_t& light, ShaderProgram* shader) {
	if (light.isCastShadow()) {
		if (auto shadowMap = m_shadowTarget.getAttachedTexture(RenderTarget::Slot::Depth))
			shadowMap->unbind();
	}
}

void PointLightShadowMapping::onShadowMapResolutionChange(float w, float h) {
	m_shadowMapResolution = { w, h };
	m_shadowViewport = Viewport_t(0.f, 0.f, w, h);
	bool ok = m_shadowTarget.resize({w, h});
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