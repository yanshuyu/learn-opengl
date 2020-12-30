#include"DeferredRenderer.h"
#include"ShaderProgamMgr.h"
#include"VertexArray.h"
#include"VertexLayoutDescription.h"
#include"FrameBuffer.h"
#include"Texture.h"
#include"PhongMaterial.h"
#include"Util.h"
#include"Renderer.h"
#include"SpotLightShadowMapping.h"
#include"DirectionalLightShadowMapping.h"
#include"PointLightShadowMapping.h"
#include<sstream>
#include<glm/gtc/type_ptr.hpp>


const std::string DeferredRenderer::s_identifier = "DeferredRenderer";


DeferredRenderer::DeferredRenderer(Renderer* renderer): RenderTechniqueBase(renderer)
, m_gBufferTarget(renderer->getRenderSize())
, m_outputTarget(renderer->getRenderSize())
, m_directionalLightUBO(nullptr)
, m_pointLightUBO(nullptr)
, m_spotLightUBO(nullptr)
, m_depthPassPipelineState()
, m_geometryPassPipelineState()
, m_shadowPassPipelineState()
, m_lightPassPipelineState()
, m_unlitPassPipelineState()
, m_cutOutPipelineState1()
, m_cutOutPipelineState2()
, m_shadowMappings() {

}

DeferredRenderer::~DeferredRenderer() {
	cleanUp();
	RENDER_TASK_EXECUTOR_DEINIT();
}


bool DeferredRenderer::intialize() {
	m_depthPassPipelineState.depthMode = DepthMode::Enable;
	m_depthPassPipelineState.depthFunc = DepthFunc::Less;
	m_depthPassPipelineState.depthMask = 1;

	m_geometryPassPipelineState.depthMode = DepthMode::Enable;
	m_geometryPassPipelineState.depthFunc = DepthFunc::LEqual;
	m_geometryPassPipelineState.depthMask = 0;

	m_shadowPassPipelineState.depthMode = DepthMode::Enable;
	m_shadowPassPipelineState.depthFunc = DepthFunc::Less;
	m_shadowPassPipelineState.depthMask = 1;
	m_shadowPassPipelineState.cullMode = CullFaceMode::Front;
	m_shadowPassPipelineState.cullFaceWindingOrder = FaceWindingOrder::CCW;

	m_unlitPassPipelineState.depthMode = DepthMode::Disable;

	m_lightPassPipelineState.depthMode = DepthMode::Disable;
	m_lightPassPipelineState.blendMode = BlendMode::Enable;
	m_lightPassPipelineState.blendSrcFactor = BlendFactor::One;
	m_lightPassPipelineState.blendDstFactor = BlendFactor::One;
	m_lightPassPipelineState.blendFunc = BlendFunc::Add;

	m_cutOutPipelineState1.depthMode = DepthMode::Enable;
	m_cutOutPipelineState1.depthFunc = DepthFunc::LEqual;
	m_cutOutPipelineState1.depthMask = 0;
	m_cutOutPipelineState1.blendMode = BlendMode::Enable;
	m_cutOutPipelineState1.blendSrcFactor = BlendFactor::One;
	m_cutOutPipelineState1.blendDstFactor = BlendFactor::One;
	m_cutOutPipelineState1.blendFunc = BlendFunc::Add;

	m_cutOutPipelineState2.depthMode = DepthMode::Enable;
	m_cutOutPipelineState2.depthFunc = DepthFunc::Less;
	m_cutOutPipelineState2.depthMask = 1;
	m_cutOutPipelineState2.blendMode = BlendMode::Disable;

	bool ok = true;
	// g-buffers
	ok = setupRenderTargets();

#ifdef _DEBUG
	ASSERT(ok);
#endif // _DEBUG
	
	if (!ok)
		return false;

	// light ubo
	m_directionalLightUBO.reset(new Buffer());
	m_directionalLightUBO->bind(Buffer::Target::UniformBuffer);
	m_directionalLightUBO->loadData(nullptr, sizeof(DirectionalLightBlock), Buffer::Usage::StaticDraw);
	m_directionalLightUBO->unbind();

	m_pointLightUBO.reset(new Buffer());
	m_pointLightUBO->bind(Buffer::Target::UniformBuffer);
	m_pointLightUBO->loadData(nullptr, sizeof(PointLightBlock), Buffer::Usage::StaticDraw);
	m_pointLightUBO->unbind();

	m_spotLightUBO.reset(new Buffer());
	m_spotLightUBO->bind(Buffer::Target::UniformBuffer);
	m_spotLightUBO->loadData(nullptr, sizeof(SpotLightBlock), Buffer::Usage::StaticDraw);
	m_spotLightUBO->unbind();

	// render task executor
	m_taskExecutors[RenderPass::DepthPass] = std::unique_ptr<RenderTaskExecutor>(new DepthPassRenderTaskExecutor(this));
	m_taskExecutors[RenderPass::GeometryPass] = std::unique_ptr<RenderTaskExecutor>(new GeometryPassRenderTaskExecutor(this));
	m_taskExecutors[RenderPass::UnlitPass] = std::unique_ptr<RenderTaskExecutor>(new UlitPassRenderTaskExecutror(this));
	m_taskExecutors[RenderPass::LightPass] = std::unique_ptr<RenderTaskExecutor>(new LightPassRenderTaskExecuter(this));
	m_taskExecutors[RenderPass::ShadowPass] = std::unique_ptr<RenderTaskExecutor>(new ShadowPassRenderTaskExecutor(this));
	m_taskExecutors[RenderPass::AmbientPass] = std::unique_ptr<RenderTaskExecutor>(new AmbientPassRenderTaskExecutor(this));

	for (auto& executor : m_taskExecutors) {
		if (!executor.second->initialize()) {
			ok = false;
#ifdef _DEBUG
			ASSERT(false);
#endif // _DEBUG
			break;
		}
	}
	
	RENDER_TASK_EXECUTOR_INIT();

	return ok;
}


void DeferredRenderer::cleanUp() {
	m_directionalLightUBO.release();
	m_pointLightUBO.release();
	m_spotLightUBO.release();

	m_shadowMappings.clear();

	m_gBufferTarget.detachAllTexture();
	m_outputTarget.detachAllTexture();
}


void DeferredRenderer::beginFrame() {
	m_renderer->pushRenderTarget(&m_gBufferTarget); // push Gbuffers target
	m_renderer->clearScreen(ClearFlags::Color | ClearFlags::Depth | ClearFlags::Stencil);
}

void DeferredRenderer::endFrame() {
	m_renderer->popRenderTarget(); // pop Gbuffers target

	for (size_t unit = size_t(Texture::Unit::Defualt); unit < size_t(Texture::Unit::MaxUnit); unit++) {
		GLCALL(glActiveTexture(GL_TEXTURE0 + unit));
		GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
	}

	GLCALL(glBindVertexArray(0));

	if (m_passShader) {
		m_passShader->unbind();
		m_passShader = nullptr;
	}
}

void DeferredRenderer::drawDepthPass(const Scene_t& scene) {
	m_pass = RenderPass::DepthPass;
	m_renderer->pushGPUPipelineState(&m_depthPassPipelineState);
	m_renderer->setColorMask(false);

	auto shaderMgr = ShaderProgramManager::getInstance();
	auto preZShader = shaderMgr->getProgram("DepthPass");
	if (preZShader.expired())
		preZShader = shaderMgr->addProgram("DepthPass.shader");
	ASSERT(!preZShader.expired());

	m_passShader = preZShader.lock();
	m_renderer->pushShaderProgram(m_passShader.get());

	// set view project matrix
	if (m_passShader->hasUniform("u_VPMat")) {
		auto& camera = *scene.mainCamera;
		glm::mat4 vp = camera.projMatrix * camera.viewMatrix;
		m_passShader->setUniformMat4v("u_VPMat", &vp[0][0]);
	}

	for (size_t i = 0; i < scene.numOpaqueItems; i++) {
		render(scene.opaqueItems[i]);
	}

	m_renderer->popGPUPipelineState();
	m_renderer->setColorMask(true);
	m_renderer->popShadrProgram();
	m_passShader->unbindSubroutineUniforms();
	m_passShader = nullptr;
	m_pass = RenderPass::None;
}


void DeferredRenderer::drawGeometryPass(const Scene_t& scene) {
	m_pass = RenderPass::GeometryPass;
	m_renderer->pushGPUPipelineState(&m_geometryPassPipelineState);

	auto geometryShader = ShaderProgramManager::getInstance()->getProgram("GeometryPass");
	if (geometryShader.expired())
		geometryShader = ShaderProgramManager::getInstance()->addProgram("GeometryPass");
	ASSERT(!geometryShader.expired());

	m_passShader = geometryShader.lock();
	m_renderer->pushShaderProgram(m_passShader.get());

	// set view project matrix
	if (m_passShader->hasUniform("u_VPMat")) {
		auto& camera = *scene.mainCamera;
		glm::mat4 vp = camera.projMatrix * camera.viewMatrix;
		m_passShader->setUniformMat4v("u_VPMat", &vp[0][0]);
	}

	for (size_t i = 0; i < scene.numOpaqueItems; i++) {
		render(scene.opaqueItems[i]);
	}
	
	m_renderer->popGPUPipelineState();
	m_renderer->popShadrProgram();
	m_passShader->unbindSubroutineUniforms();
	m_passShader = nullptr;
	m_pass = RenderPass::None;
}



void DeferredRenderer::drawOpaquePass(const Scene_t& scene) {
	m_renderer->pushRenderTarget(&m_outputTarget);
	m_renderer->clearScreen(ClearFlags::Color);
	
	drawSolidsLights(scene);
	drawSolidsAmbient(scene);
	drawCutOutsLights(scene);
	drawCutOutsAmbient(scene);

	m_renderer->popRenderTarget();
}

void DeferredRenderer::drawUnlitScene(const Scene_t& scene) {
	m_pass = RenderPass::UnlitPass;
	m_renderer->pushGPUPipelineState(&m_unlitPassPipelineState);

	auto unlitShader = ShaderProgramManager::getInstance()->getProgram("UnlitDeferred");
	if (unlitShader.expired())
		unlitShader = ShaderProgramManager::getInstance()->addProgram("UnlitDeferred.shader");
	ASSERT(!unlitShader.expired());

	m_passShader = unlitShader.lock();
	m_renderer->pushShaderProgram(m_passShader.get());

	Texture* diffuse = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 2);
	Texture* emissive = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 4);
	
#ifdef _DEBUG
	ASSERT(diffuse && emissive);
#endif // _DEBUG

	if (diffuse) {
		diffuse->bind(Texture::Unit::DiffuseMap);
		m_passShader->setUniform1("u_diffuse", int(Texture::Unit::DiffuseMap));
	}

	if (emissive) {
		emissive->bind(Texture::Unit::EmissiveMap);
		m_passShader->setUniform1("u_emissive", int(Texture::Unit::EmissiveMap));
	}
	//draw a full screen quad
	m_renderer->drawFullScreenQuad();

	m_renderer->popGPUPipelineState();
	m_renderer->popShadrProgram();
	if (diffuse) diffuse->unbind();
	if (emissive) emissive->unbind();
	m_passShader = nullptr;
	m_pass = RenderPass::None;
}


void DeferredRenderer::drawSolidsLights(const Scene_t& scene) {
	if (scene.numOpaqueItems <= 0)
		return;

	m_renderer->pushGPUPipelineState(&m_lightPassPipelineState);

	for (size_t lightIdx = 0; lightIdx < scene.numLights; lightIdx++) {
		auto& light = scene.lights[lightIdx];

		drawLightShadow(scene, light);

		Buffer* lightUBO = nullptr;
		m_pass = RenderPass::LightPass;
		switch (light.type) {
		case LightType::DirectioanalLight: {
			auto directionalLightShader = ShaderProgramManager::getInstance()->getProgram("DirectionalLightDeferred");
			if (directionalLightShader.expired())
				directionalLightShader = ShaderProgramManager::getInstance()->addProgram("DirectionalLightDeferred.shader");
			ASSERT(!directionalLightShader.expired());

			m_passShader = directionalLightShader.lock();
			m_renderer->pushShaderProgram(m_passShader.get());

			// set directional light block
			if (m_passShader->hasUniformBlock("LightBlock")) {
				static DirectionalLightBlock dlb;
				dlb.color = glm::vec4(glm::vec3(light.color), light.intensity);
				dlb.inverseDiretion = -light.direction;
				m_directionalLightUBO->bind(Buffer::Target::UniformBuffer);
				m_directionalLightUBO->loadSubData(&dlb, 0, sizeof(dlb));

				m_directionalLightUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::LightBlock));
				m_passShader->bindUniformBlock("LightBlock", ShaderProgram::UniformBlockBindingPoint::LightBlock);

				lightUBO = m_directionalLightUBO.get();
			}

			if (m_passShader->hasUniform("u_VPMat")) {
				auto& camera = *scene.mainCamera;
				glm::mat4 vp = camera.projMatrix * camera.viewMatrix;
				m_passShader->setUniformMat4v("u_VPMat", &vp[0][0]);
			}
		} break;

		case LightType::PointLight: {
			auto pointLightShader = ShaderProgramManager::getInstance()->getProgram("PointLightDeferred");
			if (pointLightShader.expired())
				pointLightShader = ShaderProgramManager::getInstance()->addProgram("PointLightDeferred.shader");
			ASSERT(!pointLightShader.expired());

			m_passShader = pointLightShader.lock();
			m_renderer->pushShaderProgram(m_passShader.get());

			// set point light block
			if (m_passShader->hasUniformBlock("LightBlock")) {
				static PointLightBlock plb;
				plb.position = glm::vec4(light.position, light.range);
				plb.color = glm::vec4(light.color, light.intensity);

				m_pointLightUBO->bind(Buffer::Target::UniformBuffer);
				m_pointLightUBO->loadSubData(&plb, 0, sizeof(plb));
				m_pointLightUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::LightBlock));
				m_passShader->bindUniformBlock("LightBlock", ShaderProgram::UniformBlockBindingPoint::LightBlock);

				lightUBO = m_pointLightUBO.get();
			}
		}break;

		case LightType::SpotLight: {
			auto spotLightShader = ShaderProgramManager::getInstance()->getProgram("SpotLightDeferred");
			if (spotLightShader.expired())
				spotLightShader = ShaderProgramManager::getInstance()->addProgram("SpotLightDeferred.shader");
			ASSERT(!spotLightShader.expired());

			m_passShader = spotLightShader.lock();
			m_renderer->pushShaderProgram(m_passShader.get());

			// set spot light block
			if (m_passShader->hasUniformBlock("LightBlock")) {
				static SpotLightBlock slb;
				slb.position = glm::vec4(light.position, light.range);
				slb.color = glm::vec4(light.color, light.intensity);
				slb.inverseDirection = -light.direction;
				slb.angles = glm::vec2(light.innerCone, light.outterCone);

				m_spotLightUBO->bind(Buffer::Target::UniformBuffer);
				m_spotLightUBO->loadSubData(&slb, 0, sizeof(slb));
				m_spotLightUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::LightBlock));
				m_passShader->bindUniformBlock("LightBlock", ShaderProgram::UniformBlockBindingPoint::LightBlock);

				lightUBO = m_spotLightUBO.get();
			}
		}break;

		default:
#ifdef _DEBUG
			ASSERT(false);
#endif // _DEBUG
			return;
		}

		m_shadowMappings[light.type]->beginRenderLight(light, m_passShader.get());

		// set camera position
		if (m_passShader->hasUniform("u_cameraPosW")) {
			glm::vec3 camPos = scene.mainCamera->position;
			m_passShader->setUniform3v("u_cameraPosW", &camPos[0]);
		}

		// set max shininess
		if (m_passShader->hasUniform("u_maxShininess")) {
			m_passShader->setUniform1("u_maxShininess", float(PhongMaterial::s_maxShininess));
		}

		// set g-buffers
		Texture* pos = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 0);
		Texture* normal = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 1);
		Texture* diffuse = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 2);
		Texture* specular = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 3);
		Texture* emr = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 4);
		Texture* mode = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 5);

#ifdef _DEBUG
		ASSERT(pos && normal && diffuse && specular && emr && mode);
#endif // _DEBUG

		if (m_passShader->hasUniform("u_PosW") && pos) {
			pos->bind(Texture::Unit::Texture0);
			m_passShader->setUniform1("u_PosW", int(Texture::Unit::Texture0));
		}

		if (m_passShader->hasUniform("u_NormalW") && normal) {
			normal->bind(Texture::Unit::NormalMap);
			m_passShader->setUniform1("u_NormalW", int(Texture::Unit::NormalMap));
		}

		if (m_passShader->hasUniform("u_Albedo") && diffuse) {
			diffuse->bind(Texture::Unit::DiffuseMap);
			m_passShader->setUniform1("u_Albedo", int(Texture::Unit::DiffuseMap));
		}

		if (m_passShader->hasUniform("u_Specular") && specular) {
			specular->bind(Texture::Unit::SpecularMap);
			m_passShader->setUniform1("u_Specular", int(Texture::Unit::SpecularMap));
		}

		if (m_passShader->hasUniform("u_EMR") && emr) {
			emr->bind(Texture::Unit::Texture1);
			m_passShader->setUniform1("u_EMR", int(Texture::Unit::Texture1));
		}

		if (m_passShader->hasUniform("u_ShadeMode") && mode) {
			mode->bind(Texture::Unit::Texture2, Texture::Target::Texture_2D);
			m_passShader->setUniform1("u_ShadeMode", int(Texture::Unit::Texture2));
		}

		m_passShader->bindSubroutineUniforms();

		m_renderer->drawFullScreenQuad();

		m_shadowMappings[light.type]->endRenderLight(light, m_passShader.get());

		if (lightUBO) lightUBO->unbind();
		if (pos) pos->unbind();
		if (normal) normal->unbind();
		if (diffuse) diffuse->unbind();
		if (specular) specular->unbind();
		if (emr) emr->unbind();
		if (mode) mode->unbind();

		m_renderer->popShadrProgram();
		m_passShader->unbindSubroutineUniforms();
		m_passShader = nullptr;
		m_pass = RenderPass::None;
	}

	m_renderer->popGPUPipelineState();
}


void DeferredRenderer::drawCutOutsLights(const Scene_t& scene) {
	if (scene.numCutOutItems <= 0)
		return;

	m_renderer->pushGPUPipelineState(&m_cutOutPipelineState1);
	
	for (size_t lightIdx = 0; lightIdx < scene.numLights; lightIdx++) {
		auto& light = scene.lights[lightIdx];
		drawLightShadow(scene, light);

		m_pass = RenderPass::LightPass;
		Buffer* lightUBO = nullptr;
		switch (light.type) {
		case LightType::DirectioanalLight: {
			auto directionalLightShader = ShaderProgramManager::getInstance()->getProgram("DirectionalLight");
			if (directionalLightShader.expired())
				directionalLightShader = ShaderProgramManager::getInstance()->addProgram("DirectionalLight.shader");
			ASSERT(!directionalLightShader.expired());

			m_passShader = directionalLightShader.lock();
			m_renderer->pushShaderProgram(m_passShader.get());
			// set directional light block
			if (m_passShader->hasUniformBlock("LightBlock")) {
				static DirectionalLightBlock dlb;
				dlb.color = glm::vec4(glm::vec3(light.color), light.intensity);
				dlb.inverseDiretion = -light.direction;
				m_directionalLightUBO->bind(Buffer::Target::UniformBuffer);
				m_directionalLightUBO->loadSubData(&dlb, 0, sizeof(dlb));

				m_directionalLightUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::LightBlock));
				m_passShader->bindUniformBlock("LightBlock", ShaderProgram::UniformBlockBindingPoint::LightBlock);

				lightUBO = m_directionalLightUBO.get();
			}
		}break;

		case LightType::PointLight: {
			auto pointLightShader = ShaderProgramManager::getInstance()->getProgram("PointLight");
			if (pointLightShader.expired())
				pointLightShader = ShaderProgramManager::getInstance()->addProgram("PointLight.shader");
			ASSERT(!pointLightShader.expired());

			m_passShader = pointLightShader.lock();
			m_renderer->pushShaderProgram(m_passShader.get());
			// set point light block
			if (m_passShader->hasUniformBlock("LightBlock")) {
				static PointLightBlock plb;
				plb.position = glm::vec4(light.position, light.range);
				plb.color = glm::vec4(light.color, light.intensity);

				m_pointLightUBO->bind(Buffer::Target::UniformBuffer);
				m_pointLightUBO->loadSubData(&plb, 0, sizeof(plb));
				m_pointLightUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::LightBlock));
				m_passShader->bindUniformBlock("LightBlock", ShaderProgram::UniformBlockBindingPoint::LightBlock);

				lightUBO = m_pointLightUBO.get();
			}

		}break;

		case LightType::SpotLight: {
			auto spotLightShader = ShaderProgramManager::getInstance()->getProgram("SpotLight");
			if (spotLightShader.expired())
				spotLightShader = ShaderProgramManager::getInstance()->addProgram("SpotLight.shader");
			ASSERT(!spotLightShader.expired());

			m_passShader = spotLightShader.lock();
			m_renderer->pushShaderProgram(m_passShader.get());
			// set spot light block
			if (m_passShader->hasUniformBlock("LightBlock")) {
				static SpotLightBlock slb;
				slb.position = glm::vec4(light.position, light.range);
				slb.color = glm::vec4(light.color, light.intensity);
				slb.inverseDirection = -light.direction;
				slb.angles = glm::vec2(light.innerCone, light.outterCone);

				m_spotLightUBO->bind(Buffer::Target::UniformBuffer);
				m_spotLightUBO->loadSubData(&slb, 0, sizeof(slb));
				m_spotLightUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::LightBlock));
				m_passShader->bindUniformBlock("LightBlock", ShaderProgram::UniformBlockBindingPoint::LightBlock);

				lightUBO = m_spotLightUBO.get();
			}
		}break;

		default:
#ifdef _DEBUG
			ASSERT(false);
#endif // _DEBUG
			break;
		}

		auto& camera = *scene.mainCamera;
		// set view project matrix
		if (m_passShader->hasUniform("u_VPMat")) {
			glm::mat4 vp = camera.projMatrix * camera.viewMatrix;
			m_passShader->setUniformMat4v("u_VPMat", &vp[0][0]);
		}

		// set camera position
		if (m_passShader->hasUniform("u_cameraPosW")) {
			m_passShader->setUniform3v("u_cameraPosW", const_cast<float*>(glm::value_ptr(camera.position)));
		}

		m_shadowMappings[light.type]->beginRenderLight(light, m_passShader.get());
		
		if (lightIdx == 0) m_renderer->pushGPUPipelineState(&m_cutOutPipelineState2);
		for (size_t i = 0; i < scene.numCutOutItems; i++) {
			render(scene.cutOutItems[i]);
		}
		if (lightIdx == 0) m_renderer->popGPUPipelineState();

		m_shadowMappings[light.type]->endRenderLight(light, m_passShader.get());

		if (lightUBO)lightUBO->unbind();
		m_passShader->unbindUniformBlock("LightBlock");
		m_renderer->popShadrProgram();
		m_passShader->unbindSubroutineUniforms();
		m_passShader = nullptr;
		m_pass = RenderPass::None;
	}

	m_renderer->popGPUPipelineState();
}


void DeferredRenderer::drawSolidsAmbient(const Scene_t& scene) {
	if (scene.numAmbientLights <= 0)
		return;

	if (scene.numCutOutItems <= 0)
		return;

	auto shader = ShaderProgramManager::getInstance()->getProgram("HemiSphericalAmibentLightDefferred");
	if (shader.expired()) {
		shader = ShaderProgramManager::getInstance()->addProgram("HemiSphericalAmibentLightDefferred");
		ASSERT(!shader.expired());
	}

	m_pass = RenderPass::AmbientPass;
	m_passShader = shader.lock();
	m_renderer->pushShaderProgram(m_passShader.get());
	m_renderer->pushGPUPipelineState(&m_lightPassPipelineState);

	Texture* normalTex = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 1);
	Texture* diffuseTex = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 2);
	ASSERT(normalTex && diffuseTex);
	normalTex->bind(Texture::Unit::NormalMap);
	m_passShader->setUniform1("u_NormalMap", int(Texture::Unit::NormalMap));
	diffuseTex->bind(Texture::Unit::DiffuseMap);
	m_passShader->setUniform1("u_DiffuseMap", int(Texture::Unit::DiffuseMap));

	m_passShader->bindSubroutineUniforms();

	for (size_t i = 0; i < scene.numAmbientLights; i++) {
		glm::vec3 skyColor = scene.ambientLights[i].color;
		glm::vec3 groundColor = scene.ambientLights[i].colorEx;
		m_passShader->setUniform3v("u_AmbientSky", &skyColor[0]);
		m_passShader->setUniform3v("u_AmbientGround", &groundColor[0]);
		m_renderer->drawFullScreenQuad();
	}

	normalTex->unbind();
	diffuseTex->unbind();
	m_renderer->popGPUPipelineState();
	m_renderer->popShadrProgram();
	m_passShader->unbindSubroutineUniforms();
	m_passShader = nullptr;
	m_pass = RenderPass::None;
}


void DeferredRenderer::drawCutOutsAmbient(const Scene_t& scene) {
	if (scene.numAmbientLights <= 0)
		return;
	
	if (scene.numCutOutItems <= 0)
		return;

	auto shader = ShaderProgramManager::getInstance()->getProgram("HemiSphericalAmbientLight");
	if (shader.expired()) {
		shader = ShaderProgramManager::getInstance()->addProgram("HemiSphericalAmbientLight");
		ASSERT(!shader.expired());
	}

	m_pass = RenderPass::AmbientPass;
	m_passShader = shader.lock();
	m_renderer->pushShaderProgram(m_passShader.get());
	m_renderer->pushGPUPipelineState(&m_cutOutPipelineState1);

	glm::mat4 vp = scene.mainCamera->projMatrix * scene.mainCamera->viewMatrix;
	m_passShader->setUniformMat4v("u_VPMat", &vp[0][0]);

	if (scene.numLights <= 0 ) m_renderer->pushGPUPipelineState(&m_cutOutPipelineState2);

	for (size_t i = 0; i < scene.numCutOutItems; i++) {
		render(scene.cutOutItems[i]);
	}

	if (scene.numLights <= 0) m_renderer->popGPUPipelineState();

	m_renderer->popGPUPipelineState();
	m_renderer->popShadrProgram();
	m_pass = RenderPass::None;
	m_passShader->unbindSubroutineUniforms();
	m_passShader = nullptr;
}


void DeferredRenderer::drawLightShadow(const Scene_t& scene, const Light_t& light) {
	m_pass = RenderPass::ShadowPass;
	m_renderer->pushGPUPipelineState(&m_shadowPassPipelineState);
	m_renderer->setColorMask(false);

	if (m_shadowMappings.find(light.type) == m_shadowMappings.end()) {
		IShadowMapping* shadowMapping = nullptr;
		if (light.type == LightType::DirectioanalLight) {
			shadowMapping = new DirectionalLightShadowMapping(m_renderer->getRenderTechnique(), m_renderer->getShadowMapResolution(), 3);
		}
		else if (light.type == LightType::PointLight) {
			shadowMapping = new PointLightShadowMapping(m_renderer->getRenderTechnique(), m_renderer->getShadowMapResolution());
		}
		else if (light.type == LightType::SpotLight) {
			shadowMapping = new SpotLightShadowMapping(m_renderer->getRenderTechnique(), m_renderer->getShadowMapResolution());
		}
		else {
			ASSERT(false);
		}
		ASSERT(shadowMapping->initialize());

		m_shadowMappings.insert(std::make_pair(light.type, std::unique_ptr<IShadowMapping>(shadowMapping)));
	}

	m_shadowMappings[light.type]->renderShadow(scene, light);

	m_renderer->popGPUPipelineState();
	m_renderer->setColorMask(true);
	m_pass = RenderPass::None;
}


void DeferredRenderer::render(const SkyBox_t& skyBox) {
	m_renderer->pushRenderTarget(&m_outputTarget);
	__super::render(skyBox);
	m_renderer->popRenderTarget();
}


void DeferredRenderer::render(const MeshRenderItem_t& task) {
	auto taskExecutor = m_taskExecutors.find(m_pass);
	if (taskExecutor == m_taskExecutors.end()) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		std::stringstream msg;
		msg << "renderer no task executor for pass: " << int(m_pass) << "\n";
		CONSOLELOG(msg.str());
		return;
	}

	taskExecutor->second->executeMeshTask(task, m_renderer->getActiveShaderProgram());
}


void DeferredRenderer::onWindowResize(float w, float h) {
	if (w <= 0 || h <= 0)
		return;

	m_gBufferTarget.resize({ w, h });
	m_outputTarget.resize({ w, h });

	Texture* dsTex = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Depth_Stencil);
	m_outputTarget.attachProxyTexture(dsTex, RenderTarget::Slot::Depth_Stencil);
}


void DeferredRenderer::onShadowMapResolutionChange(float w, float h) {
	for (auto& shadowMapping : m_shadowMappings) {
		shadowMapping.second->onShadowMapResolutionChange(w, h);
	}
}


bool DeferredRenderer::setupRenderTargets() {
	// world position
	ASSERT(m_gBufferTarget.attachTexture2D(Texture::Format::RGB32F, RenderTarget::Slot::Color, 0));
	Texture* buffer = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 0);
	buffer->bind();
	buffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
	buffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
	buffer->unbind();

	// world normal
	ASSERT(m_gBufferTarget.attachTexture2D(Texture::Format::RGB16F, RenderTarget::Slot::Color, 1));
	buffer = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 1);
	buffer->bind();
	buffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
	buffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
	buffer->unbind();

	// albedo color
	ASSERT(m_gBufferTarget.attachTexture2D(Texture::Format::RGBA8, RenderTarget::Slot::Color, 2));
	buffer = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 2);
	buffer->bind();
	buffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
	buffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
	buffer->unbind();

	// specular color
	ASSERT(m_gBufferTarget.attachTexture2D(Texture::Format::RGBA8, RenderTarget::Slot::Color, 3));
	buffer = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 3);
	buffer->bind();
	buffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
	buffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
	buffer->unbind();

	// emissive/metallic/roughness
	ASSERT(m_gBufferTarget.attachTexture2D(Texture::Format::RGB32F, RenderTarget::Slot::Color, 4));
	buffer = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 4);
	buffer->bind();
	buffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
	buffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
	buffer->unbind();

	// shading mode (1.phong 2. pbr)
	ASSERT(m_gBufferTarget.attachTexture2D(Texture::Format::R8, RenderTarget::Slot::Color, 5));
	buffer = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 5);
	buffer->bind();
	buffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
	buffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
	buffer->unbind();

	// depth/stcencil
	ASSERT(m_gBufferTarget.attachTexture2D(Texture::Format::Depth24_Stencil8, RenderTarget::Slot::Depth_Stencil));
	buffer = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Depth_Stencil);
	buffer->bind();
	buffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
	buffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
	buffer->unbind();
	
	m_gBufferTarget.setDrawLocations({ 0, 1, 2, 3, 4, 5 });
	ASSERT(m_gBufferTarget.isValid());

	Texture* dsTex = m_gBufferTarget.getAttachedTexture(RenderTarget::Slot::Depth_Stencil);
	ASSERT(m_outputTarget.attachTexture2D(Texture::Format::RGBA16F, RenderTarget::Slot::Color));
	ASSERT(m_outputTarget.attachProxyTexture(dsTex, RenderTarget::Slot::Depth_Stencil));

	return true;
}

