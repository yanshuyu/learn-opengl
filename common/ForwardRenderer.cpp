#include"ForwardRenderer.h"
#include"ShaderProgamMgr.h"
#include"CameraComponent.h"
#include"Buffer.h"
#include"Scene.h"  
#include"Renderer.h"
#include"SpotLightShadowMapping.h"
#include"DirectionalLightShadowMapping.h"
#include"PointLightShadowMapping.h"
#include<glm/gtc/type_ptr.hpp>
#include<sstream>


const std::string ForwardRenderer::s_identifier = "ForwardRenderer";


ForwardRenderer::ForwardRenderer(Renderer* renderer): RenderTechniqueBase(renderer)
, m_frameTarget(renderer->getRenderSize())
, m_directionalLightUBO(nullptr)
, m_pointLightUBO(nullptr)
, m_spotLightUBO(nullptr)
, m_shadowMappings()
, m_depthPassPipelineState()
, m_shadowPassPipelineState()
, m_lightPassPipelineState()
, m_unlitPassPipelineState() {

}

ForwardRenderer::~ForwardRenderer() {
	cleanUp();
}

bool ForwardRenderer::intialize() {
	m_depthPassPipelineState.depthMode = DepthMode::Enable;
	m_depthPassPipelineState.depthFunc = DepthFunc::Less;
	m_depthPassPipelineState.depthMask = 1;

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

	m_unlitPassPipelineState.depthMode = DepthMode::Enable;
	m_unlitPassPipelineState.depthFunc = DepthFunc::LEqual;
	m_unlitPassPipelineState.depthMask = 0;
	

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

	m_taskExecutors[RenderPass::DepthPass] = std::unique_ptr<RenderTaskExecutor>(new DepthPassRenderTaskExecutor(this));
	m_taskExecutors[RenderPass::UnlitPass] = std::unique_ptr<RenderTaskExecutor>(new UlitPassRenderTaskExecutror(this));
	m_taskExecutors[RenderPass::LightPass] = std::unique_ptr<RenderTaskExecutor>(new LightPassRenderTaskExecuter(this));
	m_taskExecutors[RenderPass::ShadowPass] = std::unique_ptr<RenderTaskExecutor>(new ShadowPassRenderTaskExecutor(this));


	for (auto& executor : m_taskExecutors) {
		if (!executor.second->initialize()) {
#ifdef _DEBUG
			ASSERT(false);
#endif // _DEBUG
			return false;
		}
	}

	//m_renderer->pushGPUPipelineState(&GPUPipelineState::s_defaultState);
	//m_renderer->setColorMask(true);

	if (!m_frameTarget.attachTexture2D(Texture::Format::RGBA, Texture::Format::RGBA, Texture::FormatDataType::UByte, RenderTarget::Slot::Color)) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG	
		return false;
	}

	if (!m_frameTarget.attachTexture2D(Texture::Format::Depth24_Stencil8, Texture::Format::Depth_Stencil, Texture::FormatDataType::UInt_24_8, RenderTarget::Slot::Depth_Stencil)) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return false;
	}
	
	if (!m_frameTarget.isValid()) {
#ifdef _DEBUG
		 ASSERT(false);
#endif // _DEBUG
		 return false;
	 }

	return true;
}


void ForwardRenderer::cleanUp() {
	m_directionalLightUBO.release();
	m_pointLightUBO.release();
	m_spotLightUBO.release();
	m_taskExecutors.clear();
	m_shadowMappings.clear();
}



void ForwardRenderer::beginFrame() {
	m_renderer->pushRenderTarget(&m_frameTarget);
	m_renderer->clearScreen(ClearFlags::Color | ClearFlags::Depth | ClearFlags::Stencil);
}


void ForwardRenderer::endFrame() {
	m_renderer->popRenderTarget();
}


void ForwardRenderer::drawDepthPass(const Scene_t& scene) {
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
	m_renderer->popShadrProgram();
	m_renderer->setColorMask(true);
	m_passShader = nullptr;
	m_pass = RenderPass::None;
}


void ForwardRenderer::drawGeometryPass(const Scene_t& scene) {

}


void ForwardRenderer::drawOpaquePass(const Scene_t& scene) {
	if (scene.numLights <= 0) {
		drawUnlitScene(scene);
	}
	else {
		for (size_t i = 0; i < scene.numLights; i++) {
			drawLightScene(scene, scene.lights[i]);
		}
	}

}


void ForwardRenderer::drawUnlitScene(const Scene_t& scene) {
	m_pass = RenderPass::UnlitPass;
	m_renderer->pushGPUPipelineState(&m_unlitPassPipelineState);

	auto unlitShader = ShaderProgramManager::getInstance()->getProgram("Unlit");
	if (unlitShader.expired())
			unlitShader = ShaderProgramManager::getInstance()->addProgram("Unlit.shader");
	ASSERT(!unlitShader.expired());

	m_passShader = unlitShader.lock();
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
	m_passShader = nullptr;
	m_pass = RenderPass::None;
}


void ForwardRenderer::drawLightScene(const Scene_t& scene, const Light_t& light) {
	if (light.isCastShadow()) {
		drawLightShadow(scene, light);
	}

	m_pass = RenderPass::LightPass;
	m_renderer->pushGPUPipelineState(&m_lightPassPipelineState);
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

	m_shadowMappings[light.type]->beginLighttingPhase(light, m_passShader.get());

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

	for (size_t i = 0; i < scene.numOpaqueItems; i++) {
		render(scene.opaqueItems[i]);
	}

	m_shadowMappings[light.type]->endLighttingPhase(light, m_passShader.get());

	if (lightUBO)
		lightUBO->unbind();

	m_passShader->unbindUniformBlock("LightBlock");
	m_renderer->popGPUPipelineState();
	m_renderer->popShadrProgram();
	m_passShader = nullptr;
	m_pass = RenderPass::None;
}


void ForwardRenderer::drawLightShadow(const Scene_t& scene, const Light_t& light) {
	m_pass = RenderPass::ShadowPass;
	m_renderer->pushGPUPipelineState(&m_shadowPassPipelineState);
	m_renderer->setColorMask(false);

	if (m_shadowMappings.find(light.type) == m_shadowMappings.end()) {
		IShadowMapping* shadowMapping = nullptr;
		if (light.type == LightType::DirectioanalLight) {
			shadowMapping = new DirectionalLightShadowMapping(m_renderer->getRenderTechnique(), m_renderer->getShadowMapResolution());
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

	m_shadowMappings[light.type]->beginShadowPhase(scene, light);
	m_shadowMappings[light.type]->endShadowPhase();

	m_renderer->popGPUPipelineState();
	m_renderer->setColorMask(true);
	m_pass = RenderPass::None;
	m_passShader = nullptr;
}




void ForwardRenderer::render(const MeshRenderItem_t& task) {
#ifdef _DEBUG
	ASSERT(m_pass != RenderPass::None && m_pass != RenderPass::GeometryPass)
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


void ForwardRenderer::onWindowResize(float w, float h) {
	bool ok = m_frameTarget.resize({ w, h });
#ifdef _DEBUG
	ASSERT(ok);
#endif // _DEBUG

}

void ForwardRenderer::onShadowMapResolutionChange(float w, float h) {
	for (auto& shadowMapping : m_shadowMappings) {
		shadowMapping.second->onShadowMapResolutionChange(w, h);
	}
}
