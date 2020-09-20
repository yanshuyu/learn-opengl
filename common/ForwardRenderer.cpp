#include"ForwardRenderer.h"
#include"ShaderProgamMgr.h"
#include"CameraComponent.h"
#include"Buffer.h"
#include"Scene.h"


ForwardRenderer::ForwardRenderer(): RenderTechnique()
, m_activeShader(nullptr)
, m_currentPass(RenderPass::None)
, m_sceneInfo()
, m_directionalLightUBO(nullptr) {

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

	m_directionalLightUBO.reset(new Buffer());
	m_directionalLightUBO->bind(Buffer::Target::UniformBuffer);
	m_directionalLightUBO->loadData(nullptr, sizeof(DirectionalLightBlock), Buffer::Usage::StaticDraw);
	m_directionalLightUBO->unbind();

	m_taskExecutors[RenderPass::DepthPass] = std::unique_ptr<RenderTaskExecutor>(new ZPassRenderTaskExecutor(RenderTaskExecutor::RendererType::Forward));
	m_taskExecutors[RenderPass::UnlitPass] = std::unique_ptr<RenderTaskExecutor>(new UlitPassRenderTaskExecutror(RenderTaskExecutor::RendererType::Forward));
	m_taskExecutors[RenderPass::LightPass] = std::unique_ptr<RenderTaskExecutor>(new LightPassRenderTaskExecuter(RenderTaskExecutor::RendererType::Forward));

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
	m_taskExecutors.clear();
}

void ForwardRenderer::prepareForSceneRenderInfo(const SceneRenderInfo_t& si) {
	m_sceneInfo = si;
}

void ForwardRenderer::beginFrame() {
	auto& camera = m_sceneInfo.camera;
	if (camera.backgrounColor != m_clearColor)
		setClearColor(camera.backgrounColor);

	if (camera.viewport != m_viewPort)
		setViewPort(camera.viewport);

	clearScrren(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
	auto preZShader = shaderMgr->getProgram("PreZDepth");
	if (!preZShader)
		preZShader = shaderMgr->addProgram(shaderMgr->getResourceAbsolutePath() + "PreZDepth.shader");

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

void ForwardRenderer::beginLightPass(const Light_t& l) {
	GLCALL(glEnable(GL_BLEND));
	GLCALL(glBlendFunc(GL_ONE, GL_ONE));
	GLCALL(glBlendEquation(GL_FUNC_ADD));

	m_currentPass = RenderPass::LightPass;
	switch (l.type) {
		case LightType::DirectioanalLight:
			beginDirectionalLightPass(l);
			break;

		default:
			ASSERT(false);
			break;
	}
}

void ForwardRenderer::endLightPass(const Light_t& l) {
	switch (l.type) {
	case LightType::DirectioanalLight:
		endDirectionalLightPass();
		break;

	default:
		ASSERT(false);
		break;
	}

	GLCALL(glDisable(GL_BLEND));
}

void ForwardRenderer::beginTransparencyPass() {

}

void ForwardRenderer::endTransparencyPass() {

}

RenderTechnique::RenderPass ForwardRenderer::currentRenderPass() const {
	return m_currentPass;
}


void ForwardRenderer::performTask(const RenderTask_t& task) {
	if (m_currentPass == RenderPass::None || m_currentPass == RenderPass::GeometryPass)
		return;

	auto taskExecutor = m_taskExecutors.find(m_currentPass);
	
	if (taskExecutor == m_taskExecutors.end()) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG

		return;
	}

	taskExecutor->second->executeTask(task, m_sceneInfo, m_activeShader);
}



void ForwardRenderer::beginDirectionalLightPass(const Light_t& l) {
	auto directionalLightShader = ShaderProgramManager::getInstance()->getProgram("DirectionalLight");
	if (!directionalLightShader)
		directionalLightShader = ShaderProgramManager::getInstance()->addProgram("res/shader/DirectionalLight.shader");
	ASSERT(directionalLightShader);

	directionalLightShader->bind();
	m_activeShader = directionalLightShader.get();

	// set camera position
	if (directionalLightShader->hasUniform("u_cameraPosW")) {
		glm::vec3 camPos = m_sceneInfo.camera.position;
		directionalLightShader->setUniform3v("u_cameraPosW", &camPos[0]);
	}

	// set directional light block
	if (directionalLightShader->hasUniformBlock("LightBlock")) {
		DirectionalLightBlock dlb;
		dlb.color = glm::vec4(glm::vec3(l.color), l.intensity);
		dlb.inverseDiretion = -l.direction;
		m_directionalLightUBO->bind(Buffer::Target::UniformBuffer);
		m_directionalLightUBO->loadSubData(&dlb, 0, sizeof(dlb));

		m_directionalLightUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::DirectionalLightBlock));
		directionalLightShader->bindUniformBlock("LightBlock", ShaderProgram::UniformBlockBindingPoint::DirectionalLightBlock);
	}
}


void ForwardRenderer::endDirectionalLightPass() {
	if (m_activeShader) {
		m_activeShader->unbindUniformBlock("LightBlock");
		m_directionalLightUBO->unbind();
		m_activeShader->unbind();
		m_activeShader = nullptr;
	}
	m_currentPass = RenderPass::None;
}