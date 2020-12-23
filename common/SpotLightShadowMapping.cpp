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
, m_shader()
, m_lightVP(1.f)
, m_shadowViewport(0.f, 0.f, shadowMapResolution.x, shadowMapResolution.y)
, m_shadowMapResolution(shadowMapResolution) {
}

SpotLightShadowMapping::~SpotLightShadowMapping() {
	cleanUp();
}

bool SpotLightShadowMapping::initialize() {
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
	m_lightVP = glm::mat4(1.f);
}

void SpotLightShadowMapping::renderShadow(const Scene_t& scene, const Light_t& light) {
	if (!light.isCastShadow())
		return;

	updateLightMatrix(light);

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
		m_shader->setUniformMat4v("u_VPMat", &m_lightVP[0][0]);
	}
	
	for (size_t i = 0; i < scene.numOpaqueItems; i++) {
		m_renderTech->render(scene.opaqueItems[i]);
	}

	for (size_t i = 0; i < scene.numCutOutItems; i++) {
		m_renderTech->render(scene.cutOutItems[i]);
	}

	renderer->popViewport();
	renderer->popRenderTarget();
	renderer->popShadrProgram();
	m_shader->unbindSubroutineUniforms();
	m_shader = nullptr;
}


void SpotLightShadowMapping::beginRenderLight(const Light_t& light, ShaderProgram* shader) {
	if (shader->hasSubroutineUniform(Shader::Type::FragmentShader, "u_shadowAtten")) {
		static std::map<ShadowType, std::string> shadowSubrotines{ {ShadowType::HardShadow, "hardShadow"},
			{ShadowType::SoftShadow, "softShadow"}, {ShadowType::NoShadow, "noShadow"} };

		shader->setSubroutineUniform(Shader::Type::FragmentShader, "u_shadowAtten", shadowSubrotines.at(light.shadowType));
	}
	
	if (shader->hasUniform("u_shadowMap")) {
		shader->setUniform1("u_hasShadowMap", int(light.isCastShadow()));
		if (light.isCastShadow()) {
			Texture* shadowMap = m_shadowTarget.getAttachedTexture(RenderTarget::Slot::Depth);
			if (shadowMap) {
				shadowMap->bind(Texture::Unit::ShadowMap);
				shader->setUniform1("u_shadowMap", int(Texture::Unit::ShadowMap));
			}
		}

		if (shader->hasUniform("u_shadowStrength"))
			shader->setUniform1("u_shadowStrength", light.shadowStrength);

		if (shader->hasUniform("u_shadowBias"))
			shader->setUniform1("u_shadowBias", light.shadowBias);

		if (shader->hasUniform("u_lightVP"))
			shader->setUniformMat4v("u_lightVP", &m_lightVP[0][0]);
	}
}


void SpotLightShadowMapping::endRenderLight(const Light_t& light, ShaderProgram* shader) {
	if (light.isCastShadow()) {
		if (auto shadowMap = m_shadowTarget.getAttachedTexture(RenderTarget::Slot::Depth))
			shadowMap->unbind();
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


void SpotLightShadowMapping::updateLightMatrix(const Light_t& light) {
	auto v = glm::lookAt(light.position, light.position + light.direction, glm::vec3(0.f, 1.f, 0.f));
	auto p = m_lightVP * glm::perspective(light.outterCone, 1.f, 0.1f, light.range * 1.5f);
	m_lightVP = p * v;
}

