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


SpotLightShadowMapping::SpotLightShadowMapping(IRenderTechnique* rt, const glm::vec2& shadowMapResolution)
: IShadowMapping(rt)
, m_shadowMapFBO(nullptr) 
, m_shadowMap(nullptr) 
, m_shadowUBO(nullptr) 
, m_shader()
, m_lightCamera()
, m_shadowViewport(0.f, 0.f, shadowMapResolution.x, shadowMapResolution.y)
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

void SpotLightShadowMapping::beginShadowPhase(const Scene_t& scene, const Light_t& light) {
	m_lightCamera = makeLightCamera(light);

	auto preZShader = ShaderProgramManager::getInstance()->getProgram("DepthPass");
	if (preZShader.expired())
		preZShader = ShaderProgramManager::getInstance()->addProgram("DepthPass.shader");
	ASSERT(!preZShader.expired());

	m_shader = preZShader.lock();
	auto renderer = m_renderTech->getRenderer();
	renderer->pushShaderProgram(m_shader.get());
	renderer->pushRenderTarget(m_shadowMapFBO.get());
	renderer->pushViewport(&m_shadowViewport);
	renderer->clearScreen(ClearFlags::Depth);

	// set view project matrix
	if (m_shader->hasUniform("u_VPMat")) {
		glm::mat4 vp = m_lightCamera.projMatrix * m_lightCamera.viewMatrix;
		m_shader->setUniformMat4v("u_VPMat", &vp[0][0]);
	}
	
	for (size_t i = 0; i < scene.numOpaqueItems; i++) {
		m_renderTech->render(scene.opaqueItems[i]);
	}
}


void SpotLightShadowMapping::endShadowPhase() {
	auto renderer = m_renderTech->getRenderer();
	renderer->popRenderTarget();
	renderer->popViewport();
	renderer->popShadrProgram();
	m_shader = nullptr;
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
	m_shadowViewport = Viewport_t(0.f, 0.f, w, h);
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

