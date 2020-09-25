#include"ForwardRenderer.h"
#include"ShaderProgamMgr.h"
#include"CameraComponent.h"
#include"Buffer.h"
#include"Scene.h"
#include"FrameBuffer.h"  
#include<sstream>


const std::string ForwardRenderer::s_identifier = "ForwardRenderer";


ForwardRenderer::ForwardRenderer(const RenderingSettings_t& settings): RenderTechnique()
, m_renderingSettings(settings)
, m_activeShader(nullptr)
, m_currentPass(RenderPass::None)
, m_sceneInfo(nullptr)
, m_directionalLightUBO(nullptr)
, m_pointLightUBO(nullptr)
, m_spotLightUBO(nullptr) {

}

ForwardRenderer::~ForwardRenderer() {
	cleanUp();
}

void ForwardRenderer::clearScrren(int flags) {
	GLCALL(glClear(flags));
}

bool ForwardRenderer::intialize() {
	GLCALL(glEnable(GL_DEPTH_TEST));
	GLCALL(glDepthFunc(GL_LESS));

	GLCALL(glEnable(GL_CULL_FACE));
	GLCALL(glCullFace(GL_BACK));
	GLCALL(glFrontFace(GL_CCW));

	if (!setupShadowMap())
		return false;

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

	bool ok = true;
	for (auto& executor : m_taskExecutors) {
		if (!executor.second->initialize()) {
			ok = false;
#ifdef _DEBUG
			ASSERT(false);
#endif // _DEBUG
			break;
		}
	}
	
	return ok;
}


void ForwardRenderer::cleanUp() {
	m_directionalLightUBO.release();
	m_pointLightUBO.release();
	m_spotLightUBO.release();
	m_taskExecutors.clear();

	m_shadowMapFBO.release();
	m_shadowMap.release();
	m_shadowUBO.release();
}

void ForwardRenderer::prepareForSceneRenderInfo(const SceneRenderInfo_t* si) {
	m_sceneInfo = si;
}


bool ForwardRenderer::shouldVisitScene() const {
	if (m_currentPass == RenderPass::GeometryPass 
		|| m_currentPass == RenderPass::None)
		return false;

	return true;
}

void ForwardRenderer::beginFrame() {
	auto& camera = m_sceneInfo->camera;
	if (camera.backgrounColor != m_clearColor)
		setClearColor(camera.backgrounColor);

	if (camera.viewport != m_viewPort)
		setViewPort(camera.viewport);

	clearScrren(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}


void ForwardRenderer::endFrame() {
	if (m_activeShader) {
		m_activeShader->unbind();
		m_activeShader = nullptr;
	}
	
	GLCALL(glBindVertexArray(0));

	for (size_t unit = size_t(Texture::Unit::Defualt); unit < size_t(Texture::Unit::MaxUnit); unit++) {
		GLCALL(glActiveTexture(GL_TEXTURE0 + unit));
		GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
	}

	GLCALL(glDepthMask(GL_TRUE));
	GLCALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
}


void ForwardRenderer::beginDepthPass() {
	GLCALL(glEnable(GL_DEPTH_TEST));
	GLCALL(glDepthFunc(GL_LESS));
	GLCALL(glDepthMask(GL_TRUE));
	GLCALL(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));

	auto shaderMgr = ShaderProgramManager::getInstance();
	auto preZShader = shaderMgr->getProgram("DepthPass");
	if (!preZShader)
		preZShader = shaderMgr->addProgram("res/shader/DepthPass.shader");

	ASSERT(preZShader);

	preZShader->bind();

	m_activeShader = preZShader.get();
	m_currentPass = RenderPass::DepthPass;
}


void ForwardRenderer::endDepthPass() {
	GLCALL(glEnable(GL_DEPTH_TEST));
	GLCALL(glDepthFunc(GL_LEQUAL));
	GLCALL(glDepthMask(GL_FALSE));
	GLCALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

	m_activeShader->unbind();
	m_activeShader = nullptr;
	m_currentPass = RenderPass::None;
}


void ForwardRenderer::beginGeometryPass() {
	m_currentPass = RenderPass::GeometryPass;
}

void ForwardRenderer::endGeometryPass() {
	m_currentPass = RenderPass::None;
}

void ForwardRenderer::beginUnlitPass() {
	auto unlitShader = ShaderProgramManager::getInstance()->getProgram("Unlit");
	if (!unlitShader)
			unlitShader = ShaderProgramManager::getInstance()->addProgram("res/shader/Unlit.shader");
	
	ASSERT(unlitShader != nullptr);

	unlitShader->bind();

	m_activeShader = unlitShader.get();
	m_currentPass = RenderPass::UnlitPass;
}

void ForwardRenderer::endUnlitPass() {
	m_activeShader->unbind();
	m_activeShader = nullptr;
	m_currentPass = RenderPass::None;
}


void ForwardRenderer::beginShadowPass(const Light_t& l) {
	auto preZShader = ShaderProgramManager::getInstance()->getProgram("DepthPass");
	if (!preZShader)
		preZShader = ShaderProgramManager::getInstance()->addProgram("res/shader/DepthPass.shader");

	ASSERT(preZShader);

	preZShader->bind();
	m_shadowMapFBO->bind();
	m_activeShader = preZShader.get();
	m_currentPass = RenderPass::ShadowPass;
	m_lightVPMat = l.shadowCamera.projMatrix * l.shadowCamera.viewMatrix;

	// depth pass disable depth writting, restore it
	GLCALL(glEnable(GL_DEPTH_TEST));
	GLCALL(glDepthFunc(GL_LESS));
	GLCALL(glDepthMask(GL_TRUE));
	GLCALL(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
	setViewPort(Viewport_t(0, 0, m_renderingSettings.shadowMapResolution.x, m_renderingSettings.shadowMapResolution.y));

	//using cull back face mode to output back face depth
	//GLCALL(glCullFace(GL_FRONT));
	//GLCALL(glEnable(GL_POLYGON_OFFSET_FILL));
	//GLCALL(glPolygonOffset(2.5f, 20.f));
	clearScrren(GL_DEPTH_BUFFER_BIT);

}

void ForwardRenderer::endShadowPass(const Light_t& l) {
	m_shadowMapFBO->unbind();
	FrameBuffer::bindDefault();

	if (m_activeShader) {
		m_activeShader->unbind();
		m_activeShader = nullptr;
	}
	m_currentPass = RenderPass::None;

	GLCALL(glEnable(GL_DEPTH_TEST));
	GLCALL(glDepthFunc(GL_LEQUAL));
	GLCALL(glDepthMask(GL_FALSE));
	GLCALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
	//GLCALL(glCullFace(GL_BACK));
	//GLCALL(glDisable(GL_POLYGON_OFFSET_FILL));
	//GLCALL(glPolygonOffset(0.f, 0.f));
	setViewPort(Viewport_t(0, 0, m_renderingSettings.renderSize.x, m_renderingSettings.renderSize.y));
}

void ForwardRenderer::beginLightPass(const Light_t& l) {
	GLCALL(glEnable(GL_BLEND));
	GLCALL(glBlendFunc(GL_ONE, GL_ONE));
	GLCALL(glBlendEquation(GL_FUNC_ADD));

	m_currentPass = RenderPass::LightPass;

	switch (l.type) {
	case LightType::DirectioanalLight: {
		auto directionalLightShader = ShaderProgramManager::getInstance()->getProgram("DirectionalLight");
		if (!directionalLightShader)
			directionalLightShader = ShaderProgramManager::getInstance()->addProgram("res/shader/DirectionalLight.shader");
		ASSERT(directionalLightShader);

		directionalLightShader->bind();
		m_activeShader = directionalLightShader.get();

		// set directional light block
		if (directionalLightShader->hasUniformBlock("LightBlock")) {
			static DirectionalLightBlock dlb;
			dlb.color = glm::vec4(glm::vec3(l.color), l.intensity);
			dlb.inverseDiretion = -l.direction;
			m_directionalLightUBO->bind(Buffer::Target::UniformBuffer);
			m_directionalLightUBO->loadSubData(&dlb, 0, sizeof(dlb));

			m_directionalLightUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::LightBlock));
			directionalLightShader->bindUniformBlock("LightBlock", ShaderProgram::UniformBlockBindingPoint::LightBlock);
		}
	}break;

	case LightType::PointLight: {
		auto pointLightShader = ShaderProgramManager::getInstance()->getProgram("PointLight");
		if (!pointLightShader)
			pointLightShader = ShaderProgramManager::getInstance()->addProgram("res/shader/PointLight.shader");
		ASSERT(pointLightShader);

		pointLightShader->bind();
		m_activeShader = pointLightShader.get();

		// set point light block
		if (pointLightShader->hasUniformBlock("LightBlock")) {
			static PointLightBlock plb;
			plb.position = glm::vec4(l.position, l.range);
			plb.color = glm::vec4(l.color, l.intensity);

			m_pointLightUBO->bind(Buffer::Target::UniformBuffer);
			m_pointLightUBO->loadSubData(&plb, 0, sizeof(plb));
			m_pointLightUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::LightBlock));
			pointLightShader->bindUniformBlock("LightBlock", ShaderProgram::UniformBlockBindingPoint::LightBlock);
		}
	}break;

	case LightType::SpotLight: {
		auto spotLightShader = ShaderProgramManager::getInstance()->getProgram("SpotLight");
		if (!spotLightShader)
			spotLightShader = ShaderProgramManager::getInstance()->addProgram("res/shader/SpotLight.shader");
		ASSERT(spotLightShader);

		spotLightShader->bind();
		m_activeShader = spotLightShader.get();

		// set spot light block
		if (spotLightShader->hasUniformBlock("LightBlock")) {
			static SpotLightBlock slb;
			slb.position = glm::vec4(l.position, l.range);
			slb.color = glm::vec4(l.color, l.intensity);
			slb.inverseDirection = -l.direction;
			slb.angles = glm::vec2(l.innerCone, l.outterCone);

			m_spotLightUBO->bind(Buffer::Target::UniformBuffer);
			m_spotLightUBO->loadSubData(&slb, 0, sizeof(slb));
			m_spotLightUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::LightBlock));
			spotLightShader->bindUniformBlock("LightBlock", ShaderProgram::UniformBlockBindingPoint::LightBlock);
		}

	}break;

	default:
#ifdef _DEBUG
			ASSERT(false);
#endif // _DEBUG
		break;
	}

	// set camera position
	if (m_activeShader->hasUniform("u_cameraPosW")) {
		glm::vec3 camPos = m_sceneInfo->camera.position;
		m_activeShader->setUniform3v("u_cameraPosW", &camPos[0]);
	}

	// set shadow map
	if (m_activeShader->hasUniform("u_shadowMap")) {
		m_activeShader->setUniform1("u_hasShadowMap", int(l.isCastShadow()));
		if (l.isCastShadow()) {
			m_shadowMap->bind(Texture::Unit::ShadowMap);
			m_activeShader->setUniform1("u_shadowMap", int(Texture::Unit::ShadowMap));

			static ShadowBlock shadowBlock;
			shadowBlock.lightVP = m_lightVPMat;
			shadowBlock.depthBias = l.shadowBias;
			shadowBlock.shadowStrength = l.shadowStrength;
			shadowBlock.shadowType = int(l.shadowType);
			m_shadowUBO->bind(Buffer::Target::UniformBuffer);
			m_shadowUBO->loadSubData(&shadowBlock, 0, sizeof(shadowBlock));
			m_shadowUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::ShadowBlock));
			m_activeShader->bindUniformBlock("ShadowBlock", ShaderProgram::UniformBlockBindingPoint::ShadowBlock);
		}
	}
}

void ForwardRenderer::endLightPass(const Light_t& l) {
	switch (l.type) {
	case LightType::DirectioanalLight:
		m_directionalLightUBO->unbind();
		break;

	case LightType::PointLight:
		m_pointLightUBO->unbind();;
		break;
	
	case LightType::SpotLight:
		m_spotLightUBO->unbind();
		break;

	default:
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		break;
	}

	m_activeShader->unbindUniformBlock("LightBlock");
	m_activeShader->unbind();
	m_activeShader = nullptr;
	m_currentPass = RenderPass::None;

	if (l.isCastShadow()) {
		m_shadowMap->unbind();
		m_shadowUBO->unbind();
	}

	GLCALL(glDisable(GL_BLEND));
	GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

void ForwardRenderer::beginTransparencyPass() {

}

void ForwardRenderer::endTransparencyPass() {

}


void ForwardRenderer::performTask(const RenderTask_t& task) {
#ifdef _DEBUG
	ASSERT(m_currentPass != RenderPass::None && m_currentPass != RenderPass::GeometryPass)
#endif // _DEBUG

	auto taskExecutor = m_taskExecutors.find(m_currentPass);
	if (taskExecutor == m_taskExecutors.end()) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		std::stringstream msg;
		msg << "renderer no task executor for pass: " << int(m_currentPass) << "\n";
		CONSOLELOG(msg.str());
		return;
	}

	taskExecutor->second->executeTask(task);
}


void ForwardRenderer::onWindowResize(float w, float h) {
	m_renderingSettings.renderSize = { w, h };
}

void ForwardRenderer::onShadowMapResolutionChange(float w, float h) {
	m_renderingSettings.shadowMapResolution = { w, h };
	setupShadowMap();
}


bool ForwardRenderer::setupShadowMap() {
	m_shadowUBO.reset(new Buffer());
	m_shadowUBO->bind(Buffer::Target::UniformBuffer);
	m_shadowUBO->loadData(nullptr, sizeof(ShadowBlock), Buffer::Usage::StaticDraw);
	m_shadowUBO->unbind();

	m_shadowMapFBO.reset(new FrameBuffer());
	m_shadowMap.reset(new Texture());

	bool success = true;
	m_shadowMap->bind();
	success = m_shadowMap->loadImage2DFromMemory(Texture::Format::Depth32, 
													Texture::Format::Depth,
													Texture::FormatDataType::Float, 
													m_renderingSettings.shadowMapResolution.x, 
													m_renderingSettings.shadowMapResolution.y,
													nullptr);
	m_shadowMap->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
	m_shadowMap->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
	m_shadowMap->setWrapMode(Texture::WrapType::S, Texture::WrapMode::Clamp_To_Border);
	m_shadowMap->setWrapMode(Texture::WrapType::T, Texture::WrapMode::Clamp_To_Border);
	m_shadowMap->setBorderColor({ 1.f, 1.f, 1.f, 1.f });
	m_shadowMap->unbind();


	if (!success)
		return false;

	m_shadowMapFBO->bind();
	m_shadowMapFBO->addTextureAttachment(m_shadowMap->getHandler(), FrameBuffer::AttachmentPoint::Depth);
	FrameBuffer::Status result = m_shadowMapFBO->checkStatus();
	m_shadowMapFBO->unbind();

	return result == FrameBuffer::Status::Ok;
}