#include"SpotLightShadowMapping.h"
#include"RenderTechnique.h"
#include"Renderer.h"
#include"Buffer.h"
#include"FrameBuffer.h"
#include"Texture.h"
#include"ShaderProgamMgr.h"
#include"Renderer.h"
#include<glm/gtx/transform.hpp>
#include"Util.h"


SpotLightShadowMapping::SpotLightShadowMapping(Renderer* renderer, const glm::vec2& shadowMapResolution)
: m_renderer(renderer)
, m_shadowMapFBO(nullptr) 
, m_shadowMap(nullptr) 
, m_shadowUBO(nullptr) 
, m_lightCamera()
, m_rendererViewPort()
, m_shadowMapResolution(shadowMapResolution) {
}

SpotLightShadowMapping::~SpotLightShadowMapping() {
	cleanUp();
}

bool SpotLightShadowMapping::initialize() {
	m_shadowUBO.reset(new Buffer());
	m_shadowUBO->bind(Buffer::Target::UniformBuffer);
	m_shadowUBO->loadData(nullptr, sizeof(ShadowBlock), Buffer::Usage::StaticDraw);
	m_shadowUBO->unbind();

	m_shadowMapFBO.reset(new FrameBuffer());
	m_shadowMap.reset(new Texture());

	bool success = true;
	m_shadowMap->bind();
	success = m_shadowMap->loadImage2DFromMemory(Texture::Format::Depth32,
		Texture::Format::Depth,
		Texture::FormatDataType::Float,
		m_shadowMapResolution.x,
		m_shadowMapResolution.y,
		nullptr);
	m_shadowMap->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
	m_shadowMap->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
	m_shadowMap->setWrapMode(Texture::WrapType::S, Texture::WrapMode::Clamp_To_Border);
	m_shadowMap->setWrapMode(Texture::WrapType::T, Texture::WrapMode::Clamp_To_Border);
	m_shadowMap->setBorderColor({ 1.f, 1.f, 1.f, 1.f });
	m_shadowMap->unbind();


	if (!success) {
		cleanUp();
		return false;
	}

	m_shadowMapFBO->bind();
	m_shadowMapFBO->addTextureAttachment(m_shadowMap->getHandler(), FrameBuffer::AttachmentPoint::Depth);
	FrameBuffer::Status result = m_shadowMapFBO->checkStatus();
	m_shadowMapFBO->setDrawBufferLocation({});
	m_shadowMapFBO->setReadBufferLocation(-1);
	m_shadowMapFBO->unbind();

	if (result != FrameBuffer::Status::Ok) {
		cleanUp();
		return false;
	}

	return true;
}

void SpotLightShadowMapping::cleanUp() {
	m_shadowMapFBO.release();
	m_shadowMap.release();
	m_shadowUBO.release();
	m_lightCamera = Camera_t();
}

void SpotLightShadowMapping::beginShadowPhase(const Light_t& light, const Camera_t& camera) {
	m_lightCamera = makeLightCamera(light);

	auto preZShader = ShaderProgramManager::getInstance()->getProgram("DepthPass");
	if (!preZShader)
		preZShader = ShaderProgramManager::getInstance()->addProgram("res/shader/DepthPass.shader");

	ASSERT(preZShader);

	preZShader->bind();
	m_shadowMapFBO->bind();
	m_shadowMapFBO->setDrawBufferLocation({});
	m_shadowMapFBO->setReadBufferLocation(-1);

	m_rendererViewPort = m_renderer->getViewport();
	m_renderer->setViewPort(Viewport_t(0, 0, m_shadowMapResolution.x, m_shadowMapResolution.y));

	//using cull back face mode to output back face depth
	m_renderer->setCullFaceMode(CullFaceMode::Front);
	m_renderer->clearScreen(ClearFlags::Depth);

	// set view project matrix
	if (preZShader->hasUniform("u_VPMat")) {
		glm::mat4 vp = m_lightCamera.projMatrix * m_lightCamera.viewMatrix;
		preZShader->setUniformMat4v("u_VPMat", &vp[0][0]);
	}
	
	m_renderer->pullingRenderTask(preZShader.get());
}


void SpotLightShadowMapping::endShadowPhase(const Light_t& light, const Camera_t& camera) {
	m_shadowMapFBO->unbind();
	FrameBuffer::bindDefault();
	m_renderer->setViewPort(m_rendererViewPort);
	m_renderer->setCullFaceMode(CullFaceMode::Back);
}


void SpotLightShadowMapping::beginLighttingPhase(const Light_t& light, ShaderProgram* shader) {
	if (shader->hasUniform("u_shadowMap")) {
		shader->setUniform1("u_hasShadowMap", int(light.isCastShadow()));
		if (light.isCastShadow()) {
			m_shadowMap->bind(Texture::Unit::ShadowMap);
			shader->setUniform1("u_shadowMap", int(Texture::Unit::ShadowMap));

			static ShadowBlock shadowBlock;
			shadowBlock.lightVP = m_lightCamera.projMatrix * m_lightCamera.viewMatrix;
			shadowBlock.depthBias = light.shadowBias;
			shadowBlock.shadowStrength = light.shadowStrength;
			shadowBlock.shadowType = int(light.shadowType);
			m_shadowUBO->bind(Buffer::Target::UniformBuffer);
			m_shadowUBO->loadSubData(&shadowBlock, 0, sizeof(shadowBlock));
			m_shadowUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::ShadowBlock));
			shader->bindUniformBlock("ShadowBlock", ShaderProgram::UniformBlockBindingPoint::ShadowBlock);
		}
	}
}


void SpotLightShadowMapping::endLighttingPhase(const Light_t& light, ShaderProgram* shader) {
	m_shadowMap->unbind();
	m_shadowUBO->unbind();
}


void SpotLightShadowMapping::onShadowMapResolutionChange(float w, float h) {
	m_shadowMapResolution = { w, h };
	initialize();
}


Camera_t SpotLightShadowMapping::makeLightCamera(const Light_t& light) {
	Camera_t cam;
	cam.viewMatrix = glm::lookAt(light.position, light.position + light.direction, glm::vec3(0.f, 1.f, 0.f));
	cam.projMatrix = glm::perspective(light.outterCone * 1.9f, 1.f, 1.f, light.range);
	cam.near = 1.f;
	cam.far = light.range;
	cam.position = light.position;
	
	return cam;
}


#ifdef _DEBUG
void SpotLightShadowMapping::visualizeShadowMap(const glm::vec2& wndSz, const glm::vec4& rect) {
	Renderer::visualizeDepthBuffer(m_shadowMap.get(), wndSz, rect, 1.f, m_lightCamera.far);
}
#endif // _DEBUG
