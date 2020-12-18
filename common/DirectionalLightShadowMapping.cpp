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


const int DirectionalLightShadowMapping::s_maxNumCascades = 4;


DirectionalLightShadowMapping::DirectionalLightShadowMapping(IRenderTechnique* rt, const glm::vec2& shadowMapResolution, const std::vector<float>& cascadeSplitPercentage)
: IShadowMapping(rt)
, m_cascadeSplitPercents(cascadeSplitPercentage)
, m_shadowMapResolution(shadowMapResolution)
, m_shadowViewport(0.f, 0.f, shadowMapResolution.x, shadowMapResolution.y)
, m_shadowTarget(shadowMapResolution)
, m_shader() {

}
	
bool DirectionalLightShadowMapping::initialize() {
	if (m_cascadeSplitPercents.size() + 1 > s_maxNumCascades) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG

		return false;
	}
	
	if (!m_shadowTarget.attchTexture2DArray(Texture::Format::Depth24, m_cascadeSplitPercents.size() + 1, RenderTarget::Slot::Depth)) {
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
		m_shader->setUniform1("u_numCascade", int(m_cascadeSplitPercents.size() + 1));

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
	m_shader = nullptr;
}


void DirectionalLightShadowMapping::beginRenderLight(const Light_t& light, ShaderProgram* shader) {

	if (shader->hasSubroutineUniform(Shader::Type::FragmentShader, "u_shadowAtten")) {
		switch (light.shadowType)
		{
		case ShadowType::NoShadow: 
			shader->setSubroutineUniforms(Shader::Type::FragmentShader, { {"u_shadowAtten", "noShadow"} });
			break;
		case ShadowType::HardShadow:
			shader->setSubroutineUniforms(Shader::Type::FragmentShader, { {"u_shadowAtten", "hardShadow"} });
			break;
		case ShadowType::SoftShadow:
			shader->setSubroutineUniforms(Shader::Type::FragmentShader, { {"u_shadowAtten", "softShadow"} });
			break;
		default:
			break;
		}	
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
	auto cascades = camera.viewFrustum.split(m_cascadeSplitPercents);
	//auto vfCorners = Frustum::FromMatrix(camera.projMatrix).getCorners();
	//ViewFrustum_t vf;
	//vf.points[ViewFrustum_t::LBN] = vfCorners[0];
	//vf.points[ViewFrustum_t::LTN] = vfCorners[1];
	//vf.points[ViewFrustum_t::RTN] = vfCorners[2];
	//vf.points[ViewFrustum_t::RBN] = vfCorners[3];

	//vf.points[ViewFrustum_t::LBF] = vfCorners[4];
	//vf.points[ViewFrustum_t::LTF] = vfCorners[5];
	//vf.points[ViewFrustum_t::RTF] = vfCorners[6];
	//vf.points[ViewFrustum_t::RBF] = vfCorners[7];

	//auto cascades = vf.split(m_cascadeSplitPercents);

	// store cascades far plane z pos in view clip space
	m_cascadeFarProjZ.clear();
	m_cascadeFarProjZ.resize(cascades.size(), 0.f);
	std::transform(cascades.begin(), cascades.end(), m_cascadeFarProjZ.begin(), [&](const ViewFrustum_t& cascade) {
		glm::vec4 projPos = camera.projMatrix * glm::vec4(0.f, 0.f, cascade.points[ViewFrustum_t::PointIndex::LTF].z, 1.f);
		return projPos.z;
	});

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


void DirectionalLightShadowMapping::_calcViewFrumstumCascades(const Light_t& light, const Camera_t& camera) {
	Frustum frustumViewSpace = Frustum::FromMatrix(camera.projMatrix);

}


#ifdef _DEBUG

void DirectionalLightShadowMapping::visualizeShadowMaps(const glm::vec2& wndSize) {
	//for (size_t i = 0; i < m_shadowMaps.size(); i++) {
	//	glm::vec4 rect(0.f);
	//	rect.x = wndSize.x * 0.15f * i;
	//	rect.y = 0;
	//	rect.z = wndSize.x * 0.15f;
	//	rect.w = rect.z;
	//	Renderer::visualizeDepthBuffer(m_shadowMaps[i].get(), wndSize, rect, m_cascadeCameras[i].near, m_cascadeCameras[i].far);
	//}
}

#endif // _DEBUG
