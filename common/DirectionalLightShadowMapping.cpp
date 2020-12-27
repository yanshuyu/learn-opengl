#include"DirectionalLightShadowMapping.h"
#include"Texture.h"
#include"RenderTechnique.h"
#include"Renderer.h"
#include"Util.h"
#include"ShaderProgamMgr.h"
#include"Buffer.h"
#include"Geometry3D.h"
#include<glm/gtx/transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<sstream>
#include"Containers.h"

const int DirectionalLightShadowMapping::s_maxNumCascades = 4;


DirectionalLightShadowMapping::DirectionalLightShadowMapping(IRenderTechnique* rt, const glm::vec2& shadowMapResolution, int numCascade)
: IShadowMapping(rt)
, m_numCascade(numCascade)
, m_shadowMapResolution(shadowMapResolution)
, m_shadowViewport(0.f, 0.f, shadowMapResolution.x, shadowMapResolution.y)
, m_shadowTarget(shadowMapResolution)
, m_shader() {

}
	
bool DirectionalLightShadowMapping::initialize() {
	if (m_numCascade > s_maxNumCascades) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG

		return false;
	}
	
	if (!m_shadowTarget.attchTexture2DArray(Texture::Format::Depth24, m_numCascade, RenderTarget::Slot::Depth)) {
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
	
	return true;
}


void DirectionalLightShadowMapping::cleanUp() {
	m_shadowTarget.detachAllTexture();
}

void DirectionalLightShadowMapping::renderShadow(const Scene_t& scene, const Light_t& light) {
	if (!light.isCastShadow())
		return;

	calcViewFrumstumCascades(light, *scene.mainCamera);
	
	auto shader = ShaderProgramManager::getInstance()->getProgram("DirectionalLightShadowPass");
	if (shader.expired())
		shader = ShaderProgramManager::getInstance()->addProgram("DirectionalLightShadowPass.shader");
	ASSERT(!shader.expired());

	Renderer* renderer = m_renderTech->getRenderer();
	m_shader = shader.lock();
	renderer->pushShaderProgram(m_shader.get());
	renderer->pushRenderTarget(&m_shadowTarget);
	renderer->pushViewport(&m_shadowViewport);
	renderer->clearScreen(ClearFlags::Depth);

	if (m_shader->hasUniform("u_numCascade"))
		m_shader->setUniform1("u_numCascade", m_numCascade);

	if (m_shader->hasUniform("u_lightVP[0]")) {
		std::vector<glm::mat4> lightsVP;
		std::for_each(m_cascadeCameras.begin(), m_cascadeCameras.end(), [&](const Camera_t& c) {
			lightsVP.push_back(c.projMatrix * c.viewMatrix);
		});
		m_shader->setUniformMat4v("u_lightVP[0]", glm::value_ptr(lightsVP[0]), lightsVP.size());
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


void DirectionalLightShadowMapping::beginRenderLight(const Light_t& light, ShaderProgram* shader) {
	if (shader->hasSubroutineUniform(Shader::Type::FragmentShader, "u_shadowAtten")) {
		static std::map<ShadowType, std::string> shadowSubrotines { {ShadowType::HardShadow, "hardShadow"},
			{ShadowType::SoftShadow, "softShadow"}, {ShadowType::NoShadow, "noShadow"} };
		
		shader->setSubroutineUniform(Shader::Type::FragmentShader, "u_shadowAtten", shadowSubrotines.at(light.shadowType));
	}

	if (shader->hasUniform("u_shadowMapArray") && light.isCastShadow()) {
		Texture* shadowMap = m_shadowTarget.getAttachedTexture(RenderTarget::Slot::Depth);
		if (shadowMap) {
			shadowMap->bind(Texture::Unit::ShadowMap, Texture::Target::Texture_2D_Array);
			shader->setUniform1("u_shadowMapArray", int(Texture::Unit::ShadowMap));
		}

		if (shader->hasUniform("u_lightVP[0]")) {
			std::vector<glm::mat4> lightsVP;
			std::for_each(m_cascadeCameras.begin(), m_cascadeCameras.end(), [&](const Camera_t& c) {
				lightsVP.push_back(c.projMatrix * c.viewMatrix);
			});
			shader->setUniformMat4v("u_lightVP[0]", glm::value_ptr(lightsVP[0]), lightsVP.size());
		}

		if (shader->hasUniform("u_cascadesFarZ[0]"))
			shader->setUniform1v("u_cascadesFarZ[0]", m_cascadeFarProjZ.data(), m_cascadeFarProjZ.size());

		if (shader->hasUniform("u_numCascade"))
			shader->setUniform1("u_numCascade", int(m_cascadeCameras.size()));

		if (shader->hasUniform("u_shadowStrength"))
			shader->setUniform1("u_shadowStrength", light.shadowStrength);

		if (shader->hasUniform("u_shadowBias"))
			shader->setUniform1("u_shadowBias", light.shadowBias);
	}
	
}

void DirectionalLightShadowMapping::endRenderLight(const Light_t& light, ShaderProgram* shader) {
	if (light.isCastShadow()) {
		Texture* shadowMap = m_shadowTarget.getAttachedTexture(RenderTarget::Slot::Depth);
		if (shadowMap)
			shadowMap->unbind();
	}
}

void DirectionalLightShadowMapping::onShadowMapResolutionChange(float w, float h) {
	m_shadowMapResolution = { w, h };
	m_shadowViewport = Viewport_t(0.f, 0.f, w, h);
	bool ok = m_shadowTarget.resize({w, h});
#ifdef _DEBUG
	ASSERT(ok);
#endif // _DEBUG
}

void DirectionalLightShadowMapping::calcViewFrumstumCascades(const Light_t& light, const Camera_t& camera) {
	//split view frumstum to cascade
	calcViewFrumstumSplitPercents(camera, 0.2);
	auto cascades = camera.viewFrustum.split(m_splitPercents);

	// transform camera view frustum cascades from view space to world space
	glm::mat4 toWorld = glm::inverse(camera.viewMatrix);
	for (auto ca = cascades.begin(); ca != cascades.end(); ca++) {
		ca->applyTransform(toWorld);
	}

	//transform view frustum cascades form world space to light space
	//calc otho projection matrix for each cascade
	m_cascadeCameras.clear();
	for (auto& cascade : cascades) {
		glm::vec3 center = cascade.getCenter();
		glm::mat4 viewMat = glm::lookAt(center, center + light.direction, glm::vec3(0.f, 1.f, 0.f));
		cascade.applyTransform(viewMat);
		AABB_t aabb = cascade.getAABB();

		glm::mat4 projMat = glm::ortho(aabb.minimum.x, aabb.maximum.x, aabb.minimum.y, aabb.maximum.y, aabb.minimum.z, aabb.maximum.z);
		Camera_t c;
		c.viewMatrix = viewMat;
		c.projMatrix = projMat;
		c.near = aabb.minimum.z;
		c.far = aabb.maximum.z;
		m_cascadeCameras.push_back(c);
	}
}


void DirectionalLightShadowMapping::calcViewFrumstumSplitPercents(const Camera_t& camera, float t) {
	m_splitPercents.clear();
	m_cascadeFarProjZ.clear();
	m_splitPercents.reserve(m_numCascade);
	m_cascadeFarProjZ.reserve(m_numCascade);

	float n = camera.near;
	float f = camera.far;
	for (int i = 0; i < m_numCascade - 1; i++) {
		float logPart = n * glm::pow(f / n, (i + 1) / float(m_numCascade));
		float linePart = n + (f - n) * (i + 1) / float(m_numCascade);
		float distance = logPart * t + linePart * (1 - t);
		m_splitPercents.push_back((distance - n) / (f - n));
		
		glm::vec4 p = camera.projMatrix * glm::vec4(0.f, 0.f, distance, 1.f);
		m_cascadeFarProjZ.push_back(p.z);
	}

	glm::vec4 p = camera.projMatrix * glm::vec4(0.f, 0.f, camera.far, 1.f);
	m_cascadeFarProjZ.push_back(p.z);
}
