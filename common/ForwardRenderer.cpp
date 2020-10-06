#include"ForwardRenderer.h"
#include"ShaderProgamMgr.h"
#include"CameraComponent.h"
#include"Buffer.h"
#include"Scene.h"
#include"FrameBuffer.h"  
#include"Renderer.h"
#include"SpotLightShadowMapping.h"
#include"DirectionalLightShadowMapping.h"
#include"PointLightShadowMapping.h"
#include<glm/gtc/type_ptr.hpp>
#include<sstream>



const std::string ForwardRenderer::s_identifier = "ForwardRenderer";


ForwardRenderer::ForwardRenderer(Renderer* invoker, const RenderingSettings_t& settings): RenderTechnique(invoker)
, m_currentPass(RenderPass::None)
, m_directionalLightUBO(nullptr)
, m_pointLightUBO(nullptr)
, m_spotLightUBO(nullptr)
, m_spotLightShadow(nullptr)
, m_dirLightShadow(nullptr)
, m_pointLightShadow(nullptr) {

}

ForwardRenderer::~ForwardRenderer() {
	cleanUp();
}

void ForwardRenderer::clearScreen(int flags) {
	GLCALL(glClear(flags));
}

bool ForwardRenderer::intialize() {
	m_invoker->setDepthTestMode(DepthTestMode::Enable);
	m_invoker->setDepthTestFunc(DepthFunc::Less);
	m_invoker->setCullFaceMode(CullFaceMode::Back);
	m_invoker->setFaceWindingOrder(FaceWindingOrder::CCW);
	
	const RenderingSettings_t* renderSetting = m_invoker->getRenderingSettings();
	bool ok = true;
	// shadow mapping
	m_spotLightShadow.reset(new SpotLightShadowMapping(m_invoker,
		{ renderSetting->shadowMapResolution.x, renderSetting->shadowMapResolution.y }));
	ok = m_spotLightShadow->initialize();
	if (!ok) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return false;
	}

	m_dirLightShadow.reset(new DirectionalLightShadowMapping(m_invoker, renderSetting->shadowMapResolution, {0.2f, 0.4f, 0.6f}));
	ok = m_dirLightShadow->initialize();
	if (!ok) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return false;
	}

	m_pointLightShadow.reset(new PointLightShadowMapping(m_invoker, renderSetting->shadowMapResolution));
	ok = m_pointLightShadow->initialize();
	if (!ok) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return false;
	}

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

	m_spotLightShadow.release();
	m_dirLightShadow.release();
	m_pointLightShadow.release();
}


bool ForwardRenderer::shouldRunPass(RenderPass pass) {
	if (pass == RenderPass::GeometryPass)
		return false;

	return true;
}


void ForwardRenderer::beginFrame() {
	clearScreen(ClearFlags::Color | ClearFlags::Depth);
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

	m_invoker->setDepthTestMode(DepthTestMode::Enable);
	m_invoker->setColorMask(true);
}


void ForwardRenderer::beginDepthPass() {
	m_invoker->setDepthTestMode(DepthTestMode::Enable);
	m_invoker->setDepthTestFunc(DepthFunc::Less);
	m_invoker->setColorMask(false);

	auto shaderMgr = ShaderProgramManager::getInstance();
	auto preZShader = shaderMgr->getProgram("DepthPass");
	if (!preZShader)
		preZShader = shaderMgr->addProgram("res/shader/DepthPass.shader");

	ASSERT(preZShader);

	preZShader->bind();

	m_activeShader = preZShader.get();
	m_currentPass = RenderPass::DepthPass;

	// set view project matrix
	if (m_activeShader->hasUniform("u_VPMat")) {
		auto& camera = m_invoker->getSceneRenderInfo()->camera;
		glm::mat4 vp = camera.projMatrix * camera.viewMatrix;
		m_activeShader->setUniformMat4v("u_VPMat", &vp[0][0]);
	}

	m_invoker->pullingRenderTask();
}


void ForwardRenderer::endDepthPass() {
	m_invoker->setDepthTestMode(DepthTestMode::ReadOnly);
	m_invoker->setDepthTestFunc(DepthFunc::LEqual);
	m_invoker->setColorMask(true);

	m_activeShader->unbind();
	m_activeShader = nullptr;
	m_currentPass = RenderPass::None;
}


void ForwardRenderer::beginGeometryPass() {
}

void ForwardRenderer::endGeometryPass() {
}

void ForwardRenderer::beginUnlitPass() {
	auto unlitShader = ShaderProgramManager::getInstance()->getProgram("Unlit");
	if (!unlitShader)
			unlitShader = ShaderProgramManager::getInstance()->addProgram("res/shader/Unlit.shader");
	
	ASSERT(unlitShader != nullptr);

	unlitShader->bind();

	m_activeShader = unlitShader.get();
	m_currentPass = RenderPass::UnlitPass;

	// set view project matrix
	if (m_activeShader->hasUniform("u_VPMat")) {
		auto& camera = m_invoker->getSceneRenderInfo()->camera;
		glm::mat4 vp = camera.projMatrix * camera.viewMatrix;
		m_activeShader->setUniformMat4v("u_VPMat", &vp[0][0]);
	}

	m_invoker->pullingRenderTask();
}

void ForwardRenderer::endUnlitPass() {
	m_activeShader->unbind();
	m_activeShader = nullptr;
	m_currentPass = RenderPass::None;
}


void ForwardRenderer::beginShadowPass(const Light_t& l) {
	m_invoker->setDepthTestMode(DepthTestMode::Enable);
	m_invoker->setDepthTestFunc(DepthFunc::Less);

	m_currentPass = RenderPass::ShadowPass;
	auto& camera = m_invoker->getSceneRenderInfo()->camera;

	switch (l.type)
	{
	case LightType::SpotLight:
		m_spotLightShadow->beginShadowPhase(l, camera);
		break;

	case LightType::DirectioanalLight:
		m_dirLightShadow->beginShadowPhase(l, camera);
		break;
	case LightType::PointLight:
		m_pointLightShadow->beginShadowPhase(l, camera);
		break;

	default:
		break;
	}
}

void ForwardRenderer::endShadowPass(const Light_t& l) {
	auto& camera = m_invoker->getSceneRenderInfo()->camera;
	switch (l.type)
	{
	case LightType::SpotLight:
		m_spotLightShadow->endShadowPhase(l);
		break;

	case LightType::DirectioanalLight:
		m_dirLightShadow->endShadowPhase(l);
		break;
	case LightType::PointLight:
		m_pointLightShadow->endShadowPhase(l);

	default:
		break;
	}

	m_invoker->setDepthTestMode(DepthTestMode::ReadOnly);
	m_invoker->setDepthTestFunc(DepthFunc::LEqual);

	if (m_activeShader) {
		m_activeShader->unbind();
		m_activeShader = nullptr;
	}
	m_currentPass = RenderPass::None;
}

void ForwardRenderer::beginLightPass(const Light_t& l) {
	m_invoker->setBlendMode(BlendMode::Enable);
	m_invoker->setBlendFactor(BlendFactor::One, BlendFactor::One);
	m_invoker->setBlendFunc(BlendFunc::Add);

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

		m_dirLightShadow->beginLighttingPhase(l, m_activeShader);

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

		m_pointLightShadow->beginLighttingPhase(l, m_activeShader);

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

		m_spotLightShadow->beginLighttingPhase(l, m_activeShader);

	}break;

	default:
#ifdef _DEBUG
			ASSERT(false);
#endif // _DEBUG
		break;
	}

	auto& camera = m_invoker->getSceneRenderInfo()->camera;
	// set view project matrix
	if (m_activeShader->hasUniform("u_VPMat")) {
		glm::mat4 vp = camera.projMatrix * camera.viewMatrix;
		m_activeShader->setUniformMat4v("u_VPMat", &vp[0][0]);
	}

	// set camera position
	if (m_activeShader->hasUniform("u_cameraPosW")) {
		m_activeShader->setUniform3v("u_cameraPosW", const_cast<float*>(glm::value_ptr(camera.position)));
	}

	m_invoker->pullingRenderTask();
}

void ForwardRenderer::endLightPass(const Light_t& l) {
	switch (l.type) {
	case LightType::DirectioanalLight:
		m_directionalLightUBO->unbind();
		m_dirLightShadow->endLighttingPhase(l, m_activeShader);
		break;

	case LightType::PointLight:
		m_pointLightUBO->unbind();;
		m_pointLightShadow->endLighttingPhase(l, m_activeShader);
		break;
	
	case LightType::SpotLight:
		m_spotLightUBO->unbind();
		m_spotLightShadow->endLighttingPhase(l, m_activeShader);
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
	m_invoker->setBlendMode(BlendMode::Disable);
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

	taskExecutor->second->executeTask(task, m_activeShader);
}


void ForwardRenderer::onWindowResize(float w, float h) {
	
}

void ForwardRenderer::onShadowMapResolutionChange(float w, float h) {
	m_spotLightShadow->onShadowMapResolutionChange(w, h);
	m_dirLightShadow->onShadowMapResolutionChange(w, h);
	m_pointLightShadow->onShadowMapResolutionChange(w, h);
}
