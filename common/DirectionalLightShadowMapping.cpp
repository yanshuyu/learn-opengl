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


const int DirectionalLightShadowMapping::s_maxNumCascades = 4;


DirectionalLightShadowMapping::DirectionalLightShadowMapping(Renderer* renderer,
	const glm::vec2& shadowMapResolution,
	const std::vector<float>& cascadeSplitPercentage) : m_renderer(renderer)
	, m_cascadeSplitPercents(cascadeSplitPercentage)
	, m_shadowMapResolution(shadowMapResolution)
	, m_FBO(nullptr)
	, m_shadowMapArray() {

}
	
bool DirectionalLightShadowMapping::initialize() {
	if (m_cascadeSplitPercents.size() + 1 > s_maxNumCascades) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG

		return false;
	}

	m_FBO.reset(new FrameBuffer());
	m_shadowMapArray.reset(new Texture());
	bool ok = true;

	m_shadowMapArray->bind(Texture::Unit::Defualt, Texture::Target::Texture_2D_Array);
	GLCALL(ok = m_shadowMapArray->loadImage2DArrayFromMemory(Texture::Format::Depth32,
														Texture::Format::Depth,
														Texture::FormatDataType::Float,
														m_shadowMapResolution.x,
														m_shadowMapResolution.y,
														m_cascadeSplitPercents.size() + 1,
														nullptr,
														false));
	if (!ok) {
		cleanUp();
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG

		return false;
	}

	m_FBO->bind();
	m_FBO->addTextureAttachment(m_shadowMapArray->getHandler(), FrameBuffer::AttachmentPoint::Depth);
	m_FBO->setDrawBufferLocation({});
	m_FBO->setReadBufferLocation(-1);
	FrameBuffer::Status status = m_FBO->checkStatus();
	m_FBO->unbind();
	m_shadowMapArray->unbind();

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
	m_shadowMapArray.release();
}

void DirectionalLightShadowMapping::beginShadowPhase(const Light_t& light, const Camera_t& camera) {
	calcViewFrumstumCascades(light, camera);
	
	auto shader = ShaderProgramManager::getInstance()->getProgram("DirectionalLightShadowPass");
	if (shader.expired())
		shader = ShaderProgramManager::getInstance()->addProgram("DirectionalLightShadowPass.shader");
	ASSERT(!shader.expired());

	std::shared_ptr<ShaderProgram> strongShader = shader.lock();
	strongShader->bind();

	m_renderer->pushRenderTarget(m_FBO.get());
	m_renderer->clearScreen(ClearFlags::Depth);
	m_rendererViewPort = m_renderer->getViewport();
	m_renderer->setViewPort(Viewport_t(0, 0, m_shadowMapResolution.x, m_shadowMapResolution.y));
	
	if (strongShader->hasUniform("u_numCascade"))
		strongShader->setUniform1("u_numCascade", int(m_cascadeSplitPercents.size() + 1));

	if (strongShader->hasUniform("u_lightVP[0]")) {
		std::vector<glm::mat4> lightsVP;
		std::for_each(m_cascadeCameras.begin(), m_cascadeCameras.end(), [&](const Camera_t& c) {
			lightsVP.push_back(c.projMatrix * c.viewMatrix);
		});
		strongShader->setUniformMat4v("u_lightVP[0]", glm::value_ptr(lightsVP[0]), lightsVP.size());
	}

	m_renderer->pullingRenderTask(shader);
}


void DirectionalLightShadowMapping::endShadowPhase(const Light_t& light) {
	m_renderer->popRenderTarget();
	m_renderer->setViewPort(m_rendererViewPort);
}

void DirectionalLightShadowMapping::beginLighttingPhase(const Light_t& light, ShaderProgram* shader) {
	
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
		m_shadowMapArray->bind(Texture::Unit::ShadowMap, Texture::Target::Texture_2D_Array);
		shader->setUniform1("u_shadowMapArray", int(Texture::Unit::ShadowMap));

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

void DirectionalLightShadowMapping::endLighttingPhase(const Light_t& light, ShaderProgram* shader) {
	if (light.isCastShadow()) {
		m_shadowMapArray->unbind();
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
	auto cascades = camera.viewFrustum.split(m_cascadeSplitPercents);

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
