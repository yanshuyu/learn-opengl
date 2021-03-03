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
, m_shadowTarget()
, m_shader(nullptr) {
	
}

PointLightShadowMapping::~PointLightShadowMapping() {
	cleanUp();
}

void PointLightShadowMapping::renderShadow(const Scene_t& scene, const Light_t& light) {
	if (!light.isCastShadow())
		return;

	m_shader = ShaderProgramManager::getInstance()->addProgram("PointLightShadowPass").lock();

	auto renderer = m_renderTech->getRenderer();
	renderer->pushShaderProgram(m_shader.get());
	renderer->pushRenderTarget(m_shadowTarget.get());
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

	for (size_t i = 0; i < scene.numCutOutItems; i++) {
		m_renderTech->render(scene.cutOutItems[i]);
	}

	for (size_t i = 0; i < scene.numTransparentItems; i++) {
		m_renderTech->render(scene.transparentItems[i]);
	}

	renderer->popViewport();
	renderer->popRenderTarget();
	renderer->popShadrProgram();
	m_shader->unbindSubroutineUniforms();
	m_shader = nullptr;
}


void PointLightShadowMapping::beginRenderLight(const Light_t& light, ShaderProgram* shader) {		
	if (shader->hasSubroutineUniform(Shader::Type::FragmentShader, "u_shadowAtten")) {
		static std::map<ShadowType, std::string> shadowSubrotines{ {ShadowType::HardShadow, "hardShadow"},
			{ShadowType::SoftShadow, "softShadow"}, {ShadowType::NoShadow, "noShadow"} };

		shader->setSubroutineUniform(Shader::Type::FragmentShader, "u_shadowAtten", shadowSubrotines.at(light.shadowType));
	}

	if (shader->hasUniform("u_shadowMap") && light.isCastShadow()) {
		Texture* shadowMap = m_shadowTarget->getAttachedTexture(RenderTarget::Slot::Depth);
		if (shadowMap) {
			shadowMap->bindToTextureUnit(Texture::Unit::ShadowMap, Texture::Target::Texture_CubeMap);
			shader->setUniform1("u_shadowMap", int(Texture::Unit::ShadowMap));
		}

		if (shader->hasUniform("u_shadowStrength"))
			shader->setUniform1("u_shadowStrength", light.shadowStrength);

		if (shader->hasUniform("u_shadowBias"))
			shader->setUniform1("u_shadowBias", light.shadowBias);
	}
}

void PointLightShadowMapping::endRenderLight(const Light_t& light, ShaderProgram* shader) {
	if (light.isCastShadow()) {
		if (auto shadowMap = m_shadowTarget->getAttachedTexture(RenderTarget::Slot::Depth))
			shadowMap->unbindFromTextureUnit();
	}
}

void PointLightShadowMapping::onShadowMapResolutionChange(float w, float h) {
	m_shadowMapResolution = { w, h };
	m_shadowViewport = Viewport_t(0.f, 0.f, w, h);
	ASSERT(setupShadowRenderTarget());
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



bool PointLightShadowMapping::setupShadowRenderTarget() {
	m_shadowTarget.reset(new RenderTarget(m_shadowMapResolution));
	m_shadowTarget->bind();

	if (!m_shadowTarget->attachTextureCube(Texture::Format::Depth24, RenderTarget::Slot::Depth)) {
		m_shadowTarget->unBind();
		return false;
	}

	if (!m_shadowTarget->isValid()) {
		m_shadowTarget->unBind();
		return false;
	}

	m_shadowTarget->unBind();

	return true;
}