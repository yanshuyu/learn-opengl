#include"ForwardPlusRenderer.h"
#include"Util.h"
#include"Buffer.h"
#include"Renderer.h"
#include"ShaderProgamMgr.h"
#include"DirectionalLightShadowMapping.h"
#include"PointLightShadowMapping.h"
#include"SpotLightShadowMapping.h"
#include<sstream>


const std::string ForwardPlusRenderer::s_identifier = "ForwarPlusRenderer";


ForwardPlusRenderer::ForwardPlusRenderer(Renderer* renderer) : RenderTechniqueBase(renderer)
, m_outputTarget(renderer->getRenderSize())
, m_taskExecutors()
, m_shadowMappings()
, m_lightsSSBO()
, m_lights() {
	RENDER_TASK_EXECUTOR_INIT();
}


ForwardPlusRenderer::~ForwardPlusRenderer() {
	cleanUp();
	RENDER_TASK_EXECUTOR_DEINIT();
}


bool ForwardPlusRenderer::intialize() {
	if (!m_outputTarget.attachTexture2D(Texture::Format::RGBA16F, RenderTarget::Slot::Color)) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG	
		return false;
	}

	if (!m_outputTarget.attachTexture2D(Texture::Format::Depth24_Stencil8, RenderTarget::Slot::Depth_Stencil)) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return false;
	}

	if (!m_outputTarget.isValid()) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return false;
	}

	m_lightsSSBO.reset(new Buffer());
	m_lightsSSBO->bind(Buffer::Target::ShaderStorageBuffer);
	ASSERT(m_lightsSSBO->loadData(nullptr, sizeof(Light) * MAX_NUM_LIGHTS, Buffer::Usage::DynamicDraw));
	m_lightsSSBO->unbind();

	m_taskExecutors[RenderPass::DepthPass] = std::unique_ptr<RenderTaskExecutor>(new DepthPassRenderTaskExecutor(this));
	m_taskExecutors[RenderPass::ShadowPass] = std::unique_ptr<RenderTaskExecutor>(new ShadowPassRenderTaskExecutor(this));
	m_taskExecutors[RenderPass::LightPass] = std::unique_ptr<RenderTaskExecutor>(new LightPassRenderTaskExecuter(this));
	m_taskExecutors[RenderPass::LightAccumulationPass] = std::unique_ptr<RenderTaskExecutor>(new LightAccumulationPassRenderTaskExecutor(this));

	for (auto& executor : m_taskExecutors) {
		if (!executor.second->initialize()) {
#ifdef _DEBUG
			ASSERT(false);
#endif // _DEBUG
			return false;
		}
	}

	m_depthPassPipelineState.depthMode = DepthMode::Enable;
	m_depthPassPipelineState.depthFunc = DepthFunc::Less;
	m_depthPassPipelineState.depthMask = 1;

	m_opaqusPipelineState.depthMode = DepthMode::Enable;
	m_opaqusPipelineState.depthFunc = DepthFunc::LEqual;
	m_opaqusPipelineState.depthMask = 0;
	m_opaqusPipelineState.blendMode = BlendMode::Disable;

	m_cutoutPipelineState.depthMode = DepthMode::Enable;
	m_cutoutPipelineState.depthFunc = DepthFunc::Less;
	m_cutoutPipelineState.depthMask = 1;
	m_cutoutPipelineState.blendMode = BlendMode::Disable;

	m_shadowPassPipelineState.depthMode = DepthMode::Enable;
	m_shadowPassPipelineState.depthFunc = DepthFunc::Less;
	m_shadowPassPipelineState.depthMask = 1;
	m_shadowPassPipelineState.cullMode = CullFaceMode::Front;
	m_shadowPassPipelineState.cullFaceWindingOrder = FaceWindingOrder::CCW;

	m_lightPassPipelineState.depthMode = DepthMode::Enable;
	m_lightPassPipelineState.depthFunc = DepthFunc::LEqual;
	m_lightPassPipelineState.depthMask = 0;
	m_lightPassPipelineState.blendMode = BlendMode::Enable;
	m_lightPassPipelineState.blendSrcFactor = BlendFactor::One;
	m_lightPassPipelineState.blendDstFactor = BlendFactor::One;
	m_lightPassPipelineState.blendFunc = BlendFunc::Add;

	return true;
}


void ForwardPlusRenderer::cleanUp() {
	m_taskExecutors.clear();
	m_shadowMappings.clear();
	m_lightsSSBO.release();
	m_outputTarget.detachAllTexture();
}


void ForwardPlusRenderer::beginFrame() {
	m_renderer->pushRenderTarget(&m_outputTarget);
	m_renderer->clearScreen(ClearFlags::Color | ClearFlags::Depth | ClearFlags::Stencil);
}


void ForwardPlusRenderer::endFrame() {
	m_renderer->popRenderTarget();
}


void ForwardPlusRenderer::drawDepthPass(const Scene_t& scene) {
	if (scene.numOpaqueItems <= 0)
		return;

	auto shaderMgr = ShaderProgramManager::getInstance();
	auto shader = shaderMgr->getProgram("DepthPass");
	if (shader.expired())
		shader = shaderMgr->addProgram("DepthPass");
	ASSERT(!shader.expired());

	m_pass = RenderPass::DepthPass;
	m_passShader = shader.lock();
	m_renderer->pushShaderProgram(m_passShader.get());
	m_renderer->pushGPUPipelineState(&m_depthPassPipelineState);
	m_renderer->setColorMask(false);
	
	// set view project matrix
	if (m_passShader->hasUniform("u_VPMat")) {
		auto& camera = *scene.mainCamera;
		glm::mat4 vp = camera.projMatrix * camera.viewMatrix;
		m_passShader->setUniformMat4v("u_VPMat", &vp[0][0]);
	}

	for (size_t i = 0; i < scene.numOpaqueItems; i++)
		render(scene.opaqueItems[i]);

	m_passShader->unbindSubroutineUniforms();
	m_renderer->popGPUPipelineState();
	m_renderer->popShadrProgram();
	m_renderer->setColorMask(true);
	m_passShader = nullptr;
	m_pass = RenderPass::None;
}


void ForwardPlusRenderer::drawOpaquePass(const Scene_t& scene) {
	DrawOpaques(scene, false);
	DrawOpaques(scene, true);
	RenderMainLights(scene);
}


void ForwardPlusRenderer::DrawOpaques(const Scene_t& scene, bool useCutout) {
	if (useCutout && scene.numCutOutItems <= 0)
		return;

	if (!useCutout && scene.numOpaqueItems <= 0)
		return;

	auto shader = ShaderProgramManager::getInstance()->getProgram("ForwardPluseShading");
	if (shader.expired())
		shader = ShaderProgramManager::getInstance()->addProgram("ForwardPluseShading");
	ASSERT(!shader.expired());
	
	m_pass = RenderPass::LightPass;
	m_passShader = shader.lock();
	m_renderer->pushShaderProgram(m_passShader.get());

	auto pipeLineState = useCutout ? &m_cutoutPipelineState : &m_opaqusPipelineState;
	m_renderer->pushGPUPipelineState(pipeLineState);

	// set view project matrix
	if (m_passShader->hasUniform("u_VPMat")) {
		glm::mat4 vp = scene.mainCamera->projMatrix * scene.mainCamera->viewMatrix;
		m_passShader->setUniformMat4v("u_VPMat", &vp[0][0]);
	}

	// set camera position
	if (m_passShader->hasUniform("u_CameraPosW")) {
		m_passShader->setUniform3v("u_CameraPosW", &scene.mainCamera->position[0]);
	}

	PrepareLights(scene);
	m_passShader->setUniform1("u_NumLights", int(scene.numLights));
	m_lightsSSBO->bindBase(Buffer::Target::ShaderStorageBuffer, 0);
	m_lightsSSBO->loadSubData(m_lights.data(), 0, sizeof(Light) * scene.numLights);
	m_passShader->bindShaderStorageBlock("Lights", 0);

	if (useCutout) {
		for (size_t i = 0; i < scene.numCutOutItems; i++)
			render(scene.cutOutItems[i]);
	}
	else {
		for (size_t i = 0; i < scene.numOpaqueItems; i++)
			render(scene.opaqueItems[i]);
	}

	m_passShader->unbindSubroutineUniforms();
	m_passShader->unbindShaderStorageBlock("Lights");
	m_lightsSSBO->unbind();
	m_renderer->popGPUPipelineState();
	m_renderer->popShadrProgram();
	m_passShader = nullptr;
	m_pass = RenderPass::None;
}


void ForwardPlusRenderer::PrepareLights(const Scene_t& scene) {
	for (size_t i = 0; i < scene.numLights; i++)
	{	
		Light_t& light = scene.lights[i];
		m_lights[i].position = glm::vec4(light.position, light.range);
		m_lights[i].direction = glm::normalize(-light.direction);
		m_lights[i].color = glm::vec4(light.color, light.intensity);
		m_lights[i].angles = light.type == LightType::Ambient ? light.colorEx : glm::vec3(light.innerCone, light.outterCone, 0.f);
		m_lights[i].type = int(light.type);
	}

}


void ForwardPlusRenderer::genShadowMap(const Scene_t& scene, const Light_t& light) {
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



void ForwardPlusRenderer::RenderMainLights(const Scene_t& scene) {
	if (scene.numMainLights <= 0)
		return;

	if ((scene.numOpaqueItems + scene.numCutOutItems) <= 0)
		return;

	m_renderer->pushGPUPipelineState(&m_lightPassPipelineState);

	for (size_t lightIdx = 0; lightIdx < scene.numMainLights; lightIdx++) {
		auto& light = scene.mainLights[lightIdx];
		genShadowMap(scene, light);

		m_pass = RenderPass::LightPass;
		switch (light.type) {
		case LightType::DirectioanalLight: {
			auto directionalLightShader = ShaderProgramManager::getInstance()->getProgram("DirectionalLight");
			if (directionalLightShader.expired())
				directionalLightShader = ShaderProgramManager::getInstance()->addProgram("DirectionalLight.shader");
			ASSERT(!directionalLightShader.expired());

			m_passShader = directionalLightShader.lock();
			m_renderer->pushShaderProgram(m_passShader.get());

			// set directional light
			glm::vec4 lightColor(light.color, light.intensity);
			glm::vec3 lightDir(-light.direction);
			m_passShader->setUniform4v("u_lightColor", &lightColor[0]);
			m_passShader->setUniform3v("u_toLight", &lightDir[0]);

		}break;

		case LightType::PointLight: {
			auto pointLightShader = ShaderProgramManager::getInstance()->getProgram("PointLight");
			if (pointLightShader.expired())
				pointLightShader = ShaderProgramManager::getInstance()->addProgram("PointLight.shader");
			ASSERT(!pointLightShader.expired());

			m_passShader = pointLightShader.lock();
			m_renderer->pushShaderProgram(m_passShader.get());

			// set point light
			glm::vec4 lightPos(light.position, light.range);
			glm::vec4 lightColor(light.color, light.intensity);
			m_passShader->setUniform4v("u_lightPos", &lightPos[0]);
			m_passShader->setUniform4v("u_lightColor", &lightColor[0]);

		}break;

		case LightType::SpotLight: {
			auto spotLightShader = ShaderProgramManager::getInstance()->getProgram("SpotLight");
			if (spotLightShader.expired())
				spotLightShader = ShaderProgramManager::getInstance()->addProgram("SpotLight.shader");
			ASSERT(!spotLightShader.expired());

			m_passShader = spotLightShader.lock();
			m_renderer->pushShaderProgram(m_passShader.get());

			// set spot light			
			glm::vec4 lightPos(light.position, light.range);
			glm::vec4 lightColor(light.color, light.intensity);
			glm::vec3 lightDir(-light.direction);
			glm::vec2 lightAngles(light.innerCone, light.outterCone);
			m_passShader->setUniform4v("u_lightPos", &lightPos[0]);
			m_passShader->setUniform4v("u_lightColor", &lightColor[0]);
			m_passShader->setUniform3v("u_toLight", &lightDir[0]);
			m_passShader->setUniform2v("u_angles", &lightAngles[0]);

		}break;

		default:
#ifdef _DEBUG
			ASSERT(false);
#endif // _DEBUG
			break;
		}

		// set view project matrix
		if (m_passShader->hasUniform("u_VPMat")) {
			glm::mat4 vp = scene.mainCamera->projMatrix * scene.mainCamera->viewMatrix;
			m_passShader->setUniformMat4v("u_VPMat", &vp[0][0]);
		}

		// set camera position
		if (m_passShader->hasUniform("u_cameraPosW")) {
			m_passShader->setUniform3v("u_cameraPosW", &scene.mainCamera->position[0]);
		}

		m_shadowMappings[light.type]->beginRenderLight(light, m_passShader.get());

		for (size_t i = 0; i < scene.numOpaqueItems; i++) {
			render(scene.opaqueItems[i]);
		}

		for (size_t i = 0; i < scene.numCutOutItems; i++) {
			render(scene.cutOutItems[i]);
		}

		m_shadowMappings[light.type]->endRenderLight(light, m_passShader.get());

		m_passShader->unbindSubroutineUniforms();
		m_renderer->popShadrProgram();
		m_passShader = nullptr;
		m_pass = RenderPass::None;
	}

	m_renderer->popGPUPipelineState();

}



void ForwardPlusRenderer::render(const MeshRenderItem_t& task) {
#ifdef _DEBUG
	ASSERT(m_pass != RenderPass::None)
#endif // _DEBUG

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



void ForwardPlusRenderer::onWindowResize(float w, float h) {
	bool ok = m_outputTarget.resize({ w, h });
#ifdef _DEBUG
	ASSERT(ok);
#endif // _DEBUG

}

void ForwardPlusRenderer::onShadowMapResolutionChange(float w, float h) {
	for (auto& shadowMapping : m_shadowMappings) {
		shadowMapping.second->onShadowMapResolutionChange(w, h);
	}
}
