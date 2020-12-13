#include"SpotLightShadowMapping.h"
#include"RenderTechnique.h"
#include"Renderer.h"
#include"Buffer.h"
#include"Texture.h"
#include"ShaderProgamMgr.h"
#include"Renderer.h"
#include<glm/gtx/transform.hpp>
#include"Util.h"


SpotLightShadowMapping::SpotLightShadowMapping(IRenderTechnique* rt, const glm::vec2& shadowMapResolution)
: IShadowMapping(rt)
, m_shadowTarget(shadowMapResolution)
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
	m_shadowUBO->loadData(nullptr, sizeof(ShadowBlock), Buffer::Usage::DynamicDraw);
	m_shadowUBO->unbind();

	if (!m_shadowTarget.attachTexture2D(Texture::Format::Depth24, RenderTarget::Slot::Depth)) {
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
		shadowMap->bind();
		shadowMap->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Liner);
		shadowMap->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Liner);
		shadowMap->setWrapMode(Texture::WrapType::S, Texture::WrapMode::Clamp_To_Border);
		shadowMap->setWrapMode(Texture::WrapType::T, Texture::WrapMode::Clamp_To_Border);
		shadowMap->setBorderColor({ 1.f, 1.f, 1.f, 1.f });
		shadowMap->setCompareMode(Texture::CompareMode::Compare_Ref_To_Texture);
		shadowMap->setCompareFunc(Texture::CompareFunc::Less);
		shadowMap->unbind();
	}

	return true;
}

void SpotLightShadowMapping::cleanUp() {
	m_shadowTarget.detachAllTexture();
	m_shadowUBO.release();
	m_lightCamera = Camera_t();
}

void SpotLightShadowMapping::renderShadow(const Scene_t& scene, const Light_t& light) {
	if (!light.isCastShadow())
		return;

	m_lightCamera = makeLightCamera(light);

	auto preZShader = ShaderProgramManager::getInstance()->getProgram("DepthPass");
	if (preZShader.expired())
		preZShader = ShaderProgramManager::getInstance()->addProgram("DepthPass.shader");
	ASSERT(!preZShader.expired());

	m_shader = preZShader.lock();
	auto renderer = m_renderTech->getRenderer();
	renderer->pushShaderProgram(m_shader.get());
	renderer->pushRenderTarget(&m_shadowTarget);
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

	renderer->popViewport();
	renderer->popRenderTarget();
	renderer->popShadrProgram();
	m_shader = nullptr;
}


void SpotLightShadowMapping::beginRenderLight(const Light_t& light, ShaderProgram* shader) {
	if (shader->hasUniform("u_shadowMap")) {
		shader->setUniform1("u_hasShadowMap", int(light.isCastShadow()));
		if (light.isCastShadow()) {
			Texture* shadowMap = m_shadowTarget.getAttachedTexture(RenderTarget::Slot::Depth);
			if (shadowMap) {
				shadowMap->bind(Texture::Unit::ShadowMap);
				shader->setUniform1("u_shadowMap", int(Texture::Unit::ShadowMap));
			}

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


void SpotLightShadowMapping::endRenderLight(const Light_t& light, ShaderProgram* shader) {
	if (light.isCastShadow()) {
		if (auto shadowMap = m_shadowTarget.getAttachedTexture(RenderTarget::Slot::Depth))
			shadowMap->unbind();

		m_shadowUBO->unbind();
	}
}


void SpotLightShadowMapping::onShadowMapResolutionChange(float w, float h) {
	m_shadowMapResolution = { w, h };
	m_shadowViewport = Viewport_t(0.f, 0.f, w, h);
	bool ok = m_shadowTarget.resize({ w, h });
#ifdef _DEBUG
	ASSERT(ok);
#endif // _DEBUGÄã»¹ÄÜ 
}


Camera_t SpotLightShadowMapping::makeLightCamera(const Light_t& light) {
	Camera_t cam;
	cam.viewMatrix = glm::lookAt(light.position, light.position + light.direction, glm::vec3(0.f, 1.f, 0.f));
	cam.projMatrix = glm::perspective(light.outterCone * 3.f, 1.f, 1.f, light.range);
	cam.near = 1.f;
	cam.far = light.range;
	cam.position = light.position;
	
	return cam;
}

