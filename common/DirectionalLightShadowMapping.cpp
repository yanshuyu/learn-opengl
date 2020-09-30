#include"DirectionalLightShadowMapping.h"
#include"FrameBuffer.h"
#include"Texture.h"
#include"RenderTechnique.h"
#include"Util.h"
#include"ShaderProgamMgr.h"
#include"Renderer.h"
#include"Buffer.h"
#include<glm/gtx/transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<sstream>


DirectionalLightShadowMapping::DirectionalLightShadowMapping(RenderTechnique* renderer,
	const glm::vec2& shadowMapResolution,
	const std::array<float, 2>& cascadeSplitPercentage) : m_renderer(renderer)
	, m_cascadeSplitPercents(cascadeSplitPercentage.begin(), cascadeSplitPercentage.end())
	, m_shadowMapResolution(shadowMapResolution)
	, m_cascades()
	, m_FBO(nullptr)
	, m_shadowMaps() {

}
	
bool DirectionalLightShadowMapping::initialize() {
	m_FBO.reset(new FrameBuffer());
	m_shadowMaps.clear();
	bool ok = true;
	for (size_t i = 0; i < m_cascadeSplitPercents.size()+1; i++){
		m_shadowMaps.emplace_back(new Texture());
		Texture* shadowMap = m_shadowMaps.back().get();
		shadowMap->bind(Texture::Unit::Defualt, Texture::Target::Texture_2D);
		ok = shadowMap->loadImage2DFromMemory(Texture::Format::Depth32,
											Texture::Format::Depth,
											Texture::FormatDataType::Float,
											m_shadowMapResolution.x,
											m_shadowMapResolution.y,
											nullptr);
		if (!ok) {
			cleanUp();
#ifdef _DEBUG
			ASSERT(false);
#endif // _DEBUG
			return false;
		}
		shadowMap->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
		shadowMap->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
		shadowMap->setWrapMode(Texture::WrapType::S, Texture::WrapMode::Clamp_To_Border);
		shadowMap->setWrapMode(Texture::WrapType::T, Texture::WrapMode::Clamp_To_Border);
		shadowMap->setWrapMode(Texture::WrapType::R, Texture::WrapMode::Clamp_To_Border);
		shadowMap->setBorderColor({ 1.f, 1.f, 1.f, 1.f });
		shadowMap->unbind();
	}

	Texture* fstShadowMap = m_shadowMaps.front().get();
	m_FBO->bind();
	m_FBO->addTextureAttachment(fstShadowMap->getHandler(), FrameBuffer::AttachmentPoint::Depth);
	FrameBuffer::Status status = m_FBO->checkStatus();
	m_FBO->unbind();

	ok = status == FrameBuffer::Status::Ok;
	if (!ok) {
		cleanUp();
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG

		return false;
	}

	return true;
}


void DirectionalLightShadowMapping::cleanUp() {
	m_FBO.release();
	m_shadowMaps.clear();
}

void DirectionalLightShadowMapping::beginShadowPhase(const Light_t& light, const Camera_t& camera) {
	calcViewFrumstumCascades(light, camera);
	
	auto preZShader = ShaderProgramManager::getInstance()->getProgram("DepthPass");
	if (!preZShader)
		preZShader = ShaderProgramManager::getInstance()->addProgram("res/shader/DepthPass.shader");

	ASSERT(preZShader);

	preZShader->bind();
	m_FBO->bind();
	m_FBO->setDrawBufferLocation({});
	m_FBO->setReadBufferLocation(-1);

	m_rendererViewPort = m_renderer->getViewport();
	m_renderer->setViewPort(Viewport_t(0, 0, m_shadowMapResolution.x, m_shadowMapResolution.y));

	//using cull back face mode to output back face depth
	GLCALL(glCullFace(GL_FRONT));

	size_t i = 0;
	for (auto& c : m_cascadeCameras) {
		m_FBO->addTextureAttachment(m_shadowMaps[i]->getHandler(), FrameBuffer::AttachmentPoint::Depth);
		
		m_renderer->clearScrren(GL_DEPTH_BUFFER_BIT);

		// set view project matrix
		if (preZShader->hasUniform("u_VPMat")) {
			glm::mat4 vp = c.projMatrix * c.viewMatrix;
			preZShader->setUniformMat4v("u_VPMat", &vp[0][0]);
		}

		m_renderer->pullingRenderTask(preZShader.get());

		i++;
	}
}


void DirectionalLightShadowMapping::endShadowPhase(const Light_t& light, const Camera_t& camera) {
	m_FBO->unbind();
	FrameBuffer::bindDefault();
	m_renderer->setViewPort(m_rendererViewPort);

	GLCALL(glCullFace(GL_BACK));
}

void DirectionalLightShadowMapping::beginLighttingPhase(const Light_t& light, ShaderProgram* shader) {
	if (shader->hasUniform("u_shadowMap[0]")) {
		shader->setUniform1("u_hasShadowMap", int(light.isCastShadow()));
		if (light.isCastShadow()) {
			static int shadowMapUnits[3] = { int(Texture::Unit::ShadowMap0), int(Texture::Unit::ShadowMap1), int(Texture::Unit::ShadowMap2) };
			m_shadowMaps[0]->bind(Texture::Unit(shadowMapUnits[0]));
			m_shadowMaps[1]->bind(Texture::Unit(shadowMapUnits[1]));
			m_shadowMaps[2]->bind(Texture::Unit(shadowMapUnits[2]));
			shader->setUniform1v("u_shadowMap[0]", &shadowMapUnits[0], 3);

			if (shader->hasUniform("u_lightVP[0]")) {
				glm::mat4 lightVPMats[3];
				lightVPMats[0] = m_cascadeCameras[0].projMatrix * m_cascadeCameras[0].viewMatrix;
				lightVPMats[1] = m_cascadeCameras[1].projMatrix * m_cascadeCameras[1].viewMatrix;
				lightVPMats[2] = m_cascadeCameras[2].projMatrix * m_cascadeCameras[2].viewMatrix;
				shader->setUniformMat4v("u_lightVP[0]", glm::value_ptr(lightVPMats[0]), 3);
			}

			if (shader->hasUniform("u_cascadesFarZ[0]"))
				shader->setUniform1v("u_cascadesFarZ[0]", m_cascadeFarProjZ.data(), 3);

			if (shader->hasUniform("u_shadowStrength"))
				shader->setUniform1("u_shadowStrength", light.shadowStrength);

			if (shader->hasUniform("u_shadowBias"))
				shader->setUniform1("u_shadowBias", light.shadowBias);

			if (shader->hasUniform("u_shadowType"))
				shader->setUniform1("u_shadowType", int(light.shadowType));
		}
	}
}

void DirectionalLightShadowMapping::endLighttingPhase(const Light_t& light, ShaderProgram* shader) {
	if (light.isCastShadow()) {
		for (auto& shadowMap : m_shadowMaps) {
			shadowMap->unbind();
		}
	}
}

void DirectionalLightShadowMapping::onShadowMapResolutionChange(float w, float h) {
	m_shadowMapResolution = { w, h };
	bool ok = initialize();
#ifdef _DEBUG
	ASSERT(ok);
#endif // _DEBUG
}

void DirectionalLightShadowMapping::calcViewFrumstumCascades(const Light_t& light, const Camera_t& camera) {
	//split view frumstum to cascade
	m_cascades = camera.viewFrustum.split(m_cascadeSplitPercents);

	// store cascades far plane z pos in view clip space
	m_cascadeFarProjZ.clear();
	m_cascadeFarProjZ.resize(m_cascades.size(), 0.f);
	std::transform(m_cascades.begin(), m_cascades.end(), m_cascadeFarProjZ.begin(), [&](const ViewFrustum_t& cascade) {
		glm::vec4 projPos = camera.projMatrix * glm::vec4(0.f, 0.f, cascade.points[ViewFrustum_t::PointIndex::LTF].z, 1.f);
		return projPos.z;
	});

	// transform camera view frustum cascades from view space to world space
	glm::mat4 toWorld = glm::inverse(camera.viewMatrix);
	for (auto cascade = m_cascades.begin(); cascade != m_cascades.end(); cascade++) {
		cascade->applyTransform(toWorld);
	}

	//transform view frustum cascades form world space to light space
	//calc otho projection matrix for each cascade
	m_cascadeCameras.clear();
	for (auto& cascade : m_cascades) {
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


#ifdef _DEBUG

void DirectionalLightShadowMapping::visualizeShadowMaps(const glm::vec2& wndSize) {
	for (size_t i = 0; i < m_shadowMaps.size(); i++) {
		glm::vec4 rect(0.f);
		rect.x = wndSize.x * 0.15f * i;
		rect.y = 0;
		rect.z = wndSize.x * 0.15f;
		rect.w = rect.z;
		Renderer::visualizeDepthBuffer(m_shadowMaps[i].get(), wndSize, rect, m_cascadeCameras[i].near, m_cascadeCameras[i].far);
	}
}

#endif // _DEBUG
