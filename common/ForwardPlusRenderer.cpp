#include"ForwardPlusRenderer.h"
#include"Util.h"
#include"Buffer.h"
#include"ShaderProgamMgr.h"
#include"DirectionalLightShadowMapping.h"
#include"PointLightShadowMapping.h"
#include"SpotLightShadowMapping.h"
#include<sstream>


const std::string ForwardPlusRenderer::s_identifier = "ForwarPlusRenderer";


ForwardPlusRenderer::ForwardPlusRenderer(Renderer* renderer) : RenderTechniqueBase(renderer)
, m_outputTarget()
, m_taskExecutors()
, m_shadowMappings()
, m_lightsSSBO()
, m_lights()
, m_isOITSetup(false)
, m_fragIdxACBO()
, m_fragListSSBO()
, m_fragListHeader()
, m_fragListHeaderResetBuffer(){
	setupPipelineStates();
	RENDER_TASK_EXECUTOR_INIT();
}


ForwardPlusRenderer::~ForwardPlusRenderer() {
	cleanUp();
	RENDER_TASK_EXECUTOR_DEINIT();
}


bool ForwardPlusRenderer::intialize() {
	if (!setupOutputTarget())
		return false;

	m_lightsSSBO.reset(new Buffer());
	m_lightsSSBO->bind(Buffer::Target::ShaderStorageBuffer);
	ASSERT(m_lightsSSBO->loadData(nullptr, sizeof(Light) * MAX_NUM_TOTAL_LIGHTS, Buffer::Usage::DynamicDraw));
	m_lightsSSBO->unbind();

	m_taskExecutors[RenderPass::DepthPass] = std::unique_ptr<RenderTaskExecutor>(new DepthPassRenderTaskExecutor(this));
	m_taskExecutors[RenderPass::ShadowPass] = std::unique_ptr<RenderTaskExecutor>(new ShadowPassRenderTaskExecutor(this));
	m_taskExecutors[RenderPass::LightPass] = std::unique_ptr<RenderTaskExecutor>(new LightPassRenderTaskExecuter(this));

	for (auto& executor : m_taskExecutors) {
		if (!executor.second->initialize()) {
#ifdef _DEBUG
			ASSERT(false);
#endif // _DEBUG
			return false;
		}
	}

	return true;
}


void ForwardPlusRenderer::cleanUp() {
	m_taskExecutors.clear();
	m_shadowMappings.clear();
	m_lightsSSBO.release();
	m_outputTarget.release();
}


void ForwardPlusRenderer::beginFrame() {
	m_renderer->pushRenderTarget(m_outputTarget.get());
	m_renderer->clearScreen(ClearFlags::Color | ClearFlags::Depth | ClearFlags::Stencil);
}


void ForwardPlusRenderer::endFrame() {
	m_renderer->popRenderTarget();
}


Texture* ForwardPlusRenderer::getRenderedFrame() {
	if (m_oitBlendTarget)
		return m_oitBlendTarget->getAttachedTexture(RenderTarget::Slot::Color);
	
	if (m_outputTarget)
		return m_outputTarget->getAttachedTexture(RenderTarget::Slot::Color);

#ifdef _DEBUG
	ASSERT(false);
#endif // _DEBUG

	return nullptr;
}

void ForwardPlusRenderer::drawDepthPass(const Scene_t& scene) {
	if (scene.numOpaqueItems <= 0)
		return;

	m_passShader = ShaderProgramManager::getInstance()->addProgram("DepthPass").lock();
	m_pass = RenderPass::DepthPass;
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
	PrepareLights(scene);
	DrawOpaques(scene, false);
	DrawOpaques(scene, true);
	RenderMainLights(scene);
}


void ForwardPlusRenderer::drawTransparentPass(const Scene_t& scene) {
	if (scene.numTransparentItems <= 0)
		return;

	if (!m_isOITSetup)
		ASSERT(setupOIT());

	genOITFragList(scene);

	m_renderer->flushDrawCommands();
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	
	blendOITFragList();
}


void ForwardPlusRenderer::DrawOpaques(const Scene_t& scene, bool useCutout) {
	if (useCutout && scene.numCutOutItems <= 0)
		return;

	if (!useCutout && scene.numOpaqueItems <= 0)
		return;

	m_passShader = ShaderProgramManager::getInstance()->addProgram("ForwardPluseShading").lock();
	m_pass = RenderPass::LightPass;
	auto pipeLineState = useCutout ? &m_cutoutPipelineState : &m_opaqusPipelineState;
	m_renderer->pushShaderProgram(m_passShader.get());
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

	m_lightsSSBO->bindBase(Buffer::Target::ShaderStorageBuffer, 1);
	m_passShader->bindShaderStorageBlock("Lights", 1);
	m_passShader->setUniform1("u_NumLights", int(scene.numLights));
	
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

	for (size_t j = 0; j < scene.numMainLights; j++) {
		Light_t& light = scene.mainLights[j];
		m_lights[j + scene.numLights].position = glm::vec4(light.position, light.range);
		m_lights[j + scene.numLights].direction = glm::normalize(-light.direction);
		m_lights[j + scene.numLights].color = glm::vec4(light.color, light.intensity);
		m_lights[j + scene.numLights].angles = light.type == LightType::Ambient ? light.colorEx : glm::vec3(light.innerCone, light.outterCone, 0.f);
		m_lights[j + scene.numLights].type = int(light.type);
	}

	m_lightsSSBO->bind(Buffer::Target::ShaderStorageBuffer);
	m_lightsSSBO->loadSubData(m_lights.data(), 0, sizeof(Light) * (scene.numLights + scene.numMainLights));
	m_lightsSSBO->unbind();
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
	ASSERT(setupOutputTarget());
	cleanOIT();
}

void ForwardPlusRenderer::onShadowMapResolutionChange(float w, float h) {
	for (auto& shadowMapping : m_shadowMappings) {
		shadowMapping.second->onShadowMapResolutionChange(w, h);
	}
}


bool ForwardPlusRenderer::setupOutputTarget() {
	m_outputTarget.reset(new RenderTarget(m_renderer->getRenderSize()));

	m_outputTarget->bind();

	if (!m_outputTarget->attachTexture2D(Texture::Format::RGBA16F, RenderTarget::Slot::Color))
		return false;

	if (!m_outputTarget->attachTexture2D(Texture::Format::Depth24_Stencil8, RenderTarget::Slot::Depth_Stencil))
		return false;

	if (!m_outputTarget->isValid())
		return false;

	m_outputTarget->unBind();

	return true;
}


void ForwardPlusRenderer::setupPipelineStates() {
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

	m_oitDrawPipelineState.depthMode = DepthMode::Enable;
	m_oitDrawPipelineState.depthFunc = DepthFunc::Less;
	m_oitDrawPipelineState.depthMask = 0;
	m_oitDrawPipelineState.blendMode = BlendMode::Disable;

	m_oitBlendPipelineState.depthMode = DepthMode::Disable;
	m_oitBlendPipelineState.cullMode = CullFaceMode::None;
	m_oitBlendPipelineState.blendMode = BlendMode::Disable;
}


bool ForwardPlusRenderer::setupOIT() {
	bool ok;
	m_fragIdxACBO.reset(new Buffer());
	m_fragIdxACBO->bind(Buffer::Target::AtomicCounterBuffer);
	ok =  m_fragIdxACBO->loadData(nullptr, sizeof(unsigned int), Buffer::Usage::DynamicDraw);
	m_fragIdxACBO->unbind();
	if (!ok) return false;

	glm::vec2 renderSz = m_renderer->getRenderSize();
	unsigned int bufSz = sizeof(Fragment) * MAX_FRAG_PER_PIXEL * renderSz.x * renderSz.y;
	m_fragListSSBO.reset(new Buffer());
	m_fragListSSBO->bind(Buffer::Target::ShaderStorageBuffer);
	ok = m_fragListSSBO->loadData(nullptr, bufSz, Buffer::Usage::DynamicDraw);
	m_fragListSSBO->unbind();
	if (!ok) return false;

	m_fragListHeader.reset(new Texture());
	m_fragListHeader->allocStorage2D(Texture::Format::R32UI, renderSz.x, renderSz.y);

	std::vector<unsigned int> clearData(renderSz.x * renderSz.y, 0xffffffff);
	m_fragListHeaderResetBuffer.reset(new Buffer());
	m_fragListHeaderResetBuffer->bind(Buffer::Target::PixelUnpackBuffer);
	ok = m_fragListHeaderResetBuffer->loadData(clearData.data(), sizeof(unsigned int) * clearData.size(), Buffer::Usage::StaticCopy);
	m_fragListHeaderResetBuffer->unbind();
	if (!ok) return false;

	m_oitBlendTarget.reset(new RenderTarget(m_renderer->getRenderSize()));
	m_oitBlendTarget->bind();
	ok = m_oitBlendTarget->attachTexture2D(Texture::Format::RGBA16F, RenderTarget::Slot::Color);
	m_isOITSetup = ok && m_oitBlendTarget->isValid();
	m_oitBlendTarget->unBind();


	return m_isOITSetup;
}


void ForwardPlusRenderer::cleanOIT() {
	m_fragIdxACBO.release();
	m_fragListSSBO.release();
	m_fragListHeaderResetBuffer.release();
	m_fragListHeader.release();
	m_oitBlendTarget.release();
	m_isOITSetup = false;
}


void ForwardPlusRenderer::genOITFragList(const Scene_t& scene) {
	m_passShader = ShaderProgramManager::getInstance()->addProgram("OITFragList").lock();
	m_pass = RenderPass::LightPass;
	m_renderer->pushShaderProgram(m_passShader.get());
	m_renderer->pushGPUPipelineState(&m_oitDrawPipelineState);
	m_renderer->setColorMask(false);

	m_fragListSSBO->bindBase(Buffer::Target::ShaderStorageBuffer, 0);
	m_passShader->bindShaderStorageBlock("FragmentBuffer", 0);

	// reset frag list header and frag index counter
	unsigned int resetIdx = 0;
	m_fragIdxACBO->bindBase(Buffer::Target::AtomicCounterBuffer, 0);
	m_fragIdxACBO->loadSubData(&resetIdx, 0, sizeof(unsigned int));

	auto renderSz = m_renderer->getRenderSize();
	//m_fragListHeader->subDataImage2DFromBuffer(m_fragListHeaderResetBuffer.get(), Texture::Format::R, Texture::FormatDataType::Int, 0, 0, renderSz.x, renderSz.y);
	m_fragListHeaderResetBuffer->bind(Buffer::Target::PixelUnpackBuffer);
	m_fragListHeader->bindToTextureUnit();
	//GLCALL(glTextureSubImage2D(m_fragListHeader->getHandler(), 0, 0, 0, renderSz.x, renderSz.y, GL_RED_INTEGER, GL_UNSIGNED_INT, 0));
	GLCALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, renderSz.x, renderSz.y, GL_RED_INTEGER, GL_UNSIGNED_INT, 0));
	m_fragListHeader->unbindFromTextureUnit();
	m_fragListHeaderResetBuffer->unbind();

	m_fragListHeader->bindToImageUnit(0, Texture::Format::R32UI, Texture::Access::ReadWrite);
	m_passShader->setUniform1("u_FragHeader", 0);

	unsigned int maxNumFrags = renderSz.x * renderSz.y * MAX_FRAG_PER_PIXEL;
	m_passShader->setUniform1("u_MaxNumFrag", maxNumFrags);

	//Lights
	int totalLights = scene.numLights + scene.numMainLights;
	m_lightsSSBO->bindBase(Buffer::Target::ShaderStorageBuffer, 1);
	m_passShader->bindShaderStorageBlock("Lights", 1);
	m_passShader->setUniform1("u_NumLights", totalLights);

	// set view project matrix
	if (m_passShader->hasUniform("u_VPMat")) {
		glm::mat4 vp = scene.mainCamera->projMatrix * scene.mainCamera->viewMatrix;
		m_passShader->setUniformMat4v("u_VPMat", &vp[0][0]);
	}

	// set camera position
	if (m_passShader->hasUniform("u_CameraPosW")) {
		m_passShader->setUniform3v("u_CameraPosW", &scene.mainCamera->position[0]);
	}

	// draw per pixel fragments link list
	for (size_t i = 0; i < scene.numTransparentItems; i++) {
		render(scene.transparentItems[i]);
	}

	m_lightsSSBO->unbind();
	m_fragListSSBO->unbind();
	m_fragIdxACBO->unbind();
	m_fragListHeader->unbindFromImageUnit();
	m_passShader->unbindShaderStorageBlock("FragmentBuffer");
	m_passShader->unbindShaderStorageBlock("Lights");
	m_passShader->unbindSubroutineUniforms();
	m_renderer->popGPUPipelineState();
	m_renderer->popShadrProgram();
	m_renderer->setColorMask(true);
	m_passShader = nullptr;
	m_pass = RenderPass::None;
}


void ForwardPlusRenderer::blendOITFragList() {
	m_passShader = ShaderProgramManager::getInstance()->addProgram("OITFragBlending").lock();
	m_renderer->pushShaderProgram(m_passShader.get());
	m_renderer->pushGPUPipelineState(&m_oitBlendPipelineState);
	m_renderer->pushRenderTarget(m_oitBlendTarget.get());

	m_fragListSSBO->bindBase(Buffer::Target::ShaderStorageBuffer, 0);
	m_passShader->bindShaderStorageBlock("FragmentBuffer", 0);

	m_fragListHeader->bindToImageUnit(0, Texture::Format::R32UI, Texture::Access::Read);
	m_passShader->setUniform1("u_FragHeader", 0);

	auto bgTex = m_outputTarget->getAttachedTexture(RenderTarget::Slot::Color);
	bgTex->bindToImageUnit(1, Texture::Format::RGBA16F, Texture::Access::Read);
	m_passShader->setUniform1("u_BackroundImage", 1);

	m_renderer->clearScreen(ClearFlags::Color);
	m_renderer->drawFullScreenQuad();

	bgTex->unbindFromImageUnit();
	m_fragListHeader->unbindFromImageUnit();
	m_fragListSSBO->unbind();
	m_passShader->unbindShaderStorageBlock("FragmentBuffer");
	m_renderer->popRenderTarget();
	m_renderer->popGPUPipelineState();
	m_renderer->popShadrProgram();
}