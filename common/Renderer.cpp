#include"Renderer.h"
#include"Util.h"
#include"Scene.h"
#include"ShaderProgamMgr.h"
#include"ForwardRenderer.h"
#include"DeferredRenderer.h"
#include"VertexArray.h"
#include"Buffer.h"
#include"Texture.h"
#include"FrameBuffer.h"
#include"VertexLayoutDescription.h"
#include"GuiMgr.h"
#include<glm/gtx/transform.hpp>
#include<functional>


Renderer::Renderer(const RenderingSettings_t& settings, Mode mode) : m_pipelineStates()
, m_renderTargets()
, m_shaders()
, m_viewports()
, m_renderingSettings(settings)
, m_renderMode(Mode::None)
, m_renderTechnique(nullptr)
, m_renderContext()
, m_clearColor({0.f, 0.f, 0.f, 1.f})
, m_clearDepth(1.f)
, m_clearStencil(0.f)
, m_scene(nullptr)
, m_sceneRenderInfo()
, m_quadVAO(nullptr)
, m_quadVBO(nullptr)
, m_quadIBO(nullptr) {
	if (mode != Mode::None)
		setRenderMode(mode);
	m_renderContext.setRenderer(this);
}

Renderer::~Renderer() {
	clenUp();
	m_quadVAO.release();
	m_quadVBO.release();
	m_quadIBO.release();
}


bool Renderer::initialize() {
	return setupFullScreenQuad();
}


void Renderer::clenUp() {
	if (m_renderTechnique) {
		m_renderTechnique->cleanUp();
		m_renderTechnique.release();
		m_renderMode = Mode::None;
	}

	while (!m_pipelineStates.empty()){
		m_pipelineStates.pop();
	}

	while (!m_renderTargets.empty()) {
		m_renderTargets.pop();
	}

	while (!m_shaders.empty()) {
		m_shaders.pop();
	}

	while (!m_viewports.empty()) {
		m_viewports.pop();
	}

	m_mainViewport = Viewport_t();
}


bool Renderer::setRenderMode(Mode mode) {
	if (m_renderMode == mode)
		return true;

	clenUp();

	if (mode == Mode::Forward) {
		m_renderTechnique.reset(new ForwardRenderer(this, m_renderingSettings));
		if (!m_renderTechnique->intialize()) {
			m_renderTechnique.release();
			m_renderMode = Mode::None;
			return false;
		}

		m_renderMode = mode;
		return true;

	} else if (mode == Mode::Deferred) {	
		m_renderTechnique.reset(new DeferredRenderer(this, m_renderingSettings));
		if (!m_renderTechnique->intialize()) {
			m_renderTechnique.release();
			m_renderMode = mode;
			return false;
		}
		
		m_renderMode = mode;
		return true;
	}
	
	m_renderMode = Mode::None;
	return true;
}


void Renderer::setShadowMapResolution(float w, float h) {
	m_renderingSettings.shadowMapResolution = { w, h };
	m_renderTechnique->onShadowMapResolutionChange(w, h);
}


Renderer::Mode Renderer::getRenderMode() const {
	return m_renderMode;
}


bool Renderer::isValid() const {
	if (m_renderMode == Mode::None)
		return false;

	return m_renderTechnique != nullptr;
}


void Renderer::pushGPUPipelineState(GPUPipelineState* pipeLineState) {
	if (pipeLineState)
		m_pipelineStates.push(pipeLineState);
	
	const GPUPipelineState& s = m_pipelineStates.empty() ? GPUPipelineState::s_defaultState : *m_pipelineStates.top();
	setGPUPipelineState(s);
}

void Renderer::popGPUPipelineState() {
	if (m_pipelineStates.empty())
		return;

	m_pipelineStates.pop();
	
	const GPUPipelineState& s = m_pipelineStates.empty() ? GPUPipelineState::s_defaultState : *m_pipelineStates.top();
	setGPUPipelineState(s);
}

void Renderer::pushRenderTarget(FrameBuffer* target) {
	if (target) {
		m_renderTargets.push(target);
		target->bind();
	}
}

void Renderer::popRenderTarget() {
	if (m_renderTargets.empty())
		return;
	
	m_renderTargets.pop();
	if (m_renderTargets.empty()) {
		GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	} else {
		m_renderTargets.top()->bind();
	}
}


void Renderer::pushShaderProgram(ShaderProgram* shader) {
	if (shader) {
		m_shaders.push(shader);
	}

	if (!m_shaders.empty()) {
		m_shaders.top()->bind();
	} else {
		GLCALL(glUseProgram(0));
	}
}

void Renderer::popShadrProgram() {
	if (m_shaders.empty())
		return;
	
	m_shaders.pop();

	if (!m_shaders.empty()) {
		m_shaders.top()->bind();
	}
	else {
		GLCALL(glUseProgram(0));
	}
}

ShaderProgram* Renderer::getActiveShaderProgram() const {
	if (m_shaders.empty())
		return nullptr;
	
	return m_shaders.top();
}

void Renderer::pushViewport(Viewport_t* viewport) {
	if (viewport)
		m_viewports.push(viewport);

	if (!m_viewports.empty()) {
		setViewPort(*m_viewports.top());
	}
}

void Renderer::popViewport() {
	if (m_viewports.empty())
		return;

	m_viewports.pop();
	
	if (!m_viewports.empty()) {
		setViewPort(*m_viewports.top());
	}
}

Viewport_t* Renderer::getActiveViewport() const {
	if (m_viewports.empty())
		return nullptr;

	return m_viewports.top();
}

void Renderer::onWindowResize(float w, float h) {
	m_renderingSettings.renderSize = { w, h };
	m_renderTechnique->onWindowResize(w, h);
}


void Renderer::renderScene(Scene* s) {
	m_scene = s;
	m_sceneRenderInfo = s->getSceneRenderInfo();
	
	if (m_clearColor != m_sceneRenderInfo->camera.backgrounColor) {
		setClearColor(m_sceneRenderInfo->camera.backgrounColor);
	}
	if (m_mainViewport != m_sceneRenderInfo->camera.viewport) {
		m_mainViewport = m_sceneRenderInfo->camera.viewport;
		popViewport();
		pushViewport(&m_mainViewport);
	}

	m_renderTechnique->beginFrame();

	// pre-z pass
	if (m_renderTechnique->shouldRunPass(RenderPass::DepthPass)) {
		m_renderContext.clearMatrix();
		m_renderTechnique->beginDepthPass();
		m_renderTechnique->endDepthPass();
	}

	// G pass
	if (m_renderTechnique->shouldRunPass(RenderPass::GeometryPass)) {
		m_renderContext.clearMatrix();
		m_renderTechnique->beginGeometryPass();
		m_renderTechnique->endGeometryPass();
	}

	// ulit pass
	if (m_sceneRenderInfo->lights.empty()) {
		if (m_renderTechnique->shouldRunPass(RenderPass::UnlitPass)) {
			m_renderContext.clearMatrix();
			m_renderTechnique->beginUnlitPass();
			m_renderTechnique->endUnlitPass();
		}
	} else {
		// light passes
		if (m_renderTechnique->shouldRunPass(RenderPass::LightPass)) {
			for (const auto& l : m_sceneRenderInfo->lights) {
				if (l.intensity <= 0)
					continue;
				
				if (l.isCastShadow() && m_renderTechnique->shouldRunPass(RenderPass::ShadowPass)) {
					m_renderContext.clearMatrix();
					m_renderTechnique->beginShadowPass(l);
					m_renderTechnique->endShadowPass(l);
				}

				m_renderContext.clearMatrix();
				m_renderTechnique->beginLightPass(l);
				m_renderTechnique->endLightPass(l);
			}
		}
	}

	// transparency pass

	m_renderTechnique->endFrame();

	// Gui
	GuiManager::getInstance()->render();
}

void Renderer::renderTask(const RenderTask_t& task) {
	m_renderTechnique->performTask(task);
}

void Renderer::pullingRenderTask() {
	if (m_scene) {
		m_renderContext.clearMatrix();
		m_scene->render(&m_renderContext);
	}
}


void Renderer::setClearColor(const glm::vec4& color) {
	m_clearColor = color;
	GLCALL(glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a));
}


void Renderer::setClearDepth(float d) {
	m_clearDepth = d;
	GLCALL(glClearDepth(m_clearDepth));
}


void Renderer::setClearStencil(int m) {
	m_clearStencil = m;
	GLCALL(glClearStencil(m_clearStencil));
}


void Renderer::clearScreen(int flags) {
	GLCALL(glClear(flags));
}

void Renderer::setCullFaceMode(CullFaceMode mode) {
	switch (mode)
	{
	case CullFaceMode::None:
		GLCALL(glDisable(GL_CULL_FACE));
		break;

	case CullFaceMode::Front:
	case CullFaceMode::Back:
	case CullFaceMode::Both:
		GLCALL(glEnable(GL_CULL_FACE));
		GLCALL(glCullFace(int(mode)));
		break;

	default:
		break;
	}
}

void Renderer::setFaceWindingOrder(FaceWindingOrder order) {
	GLCALL(glFrontFace(int(order)));
}

void Renderer::setShadeMode(ShadeMode mode) {
	GLCALL(glShadeModel(int(mode)));
}

void Renderer::setFillMode(FillMode mode) {
	GLCALL(glPolygonMode(GL_FRONT_AND_BACK, int(mode)));
}

void Renderer::setDepthMode(DepthMode mode) {
	if (mode == DepthMode::Enable) {
		GLCALL(glEnable(GL_DEPTH_TEST));
	} else {
		GLCALL(glDisable(GL_DEPTH_TEST));
	}
}

void Renderer::setDepthFunc(DepthFunc func) {
	GLCALL(glDepthFunc(int(func)));
}

void Renderer::setDepthMask(bool writable) {
	GLCALL(glDepthMask(writable));
}

void Renderer::setStencilMode(StencilMode mode) {
	if (mode == StencilMode::Enable) {
		GLCALL(glEnable(GL_STENCIL_TEST));
	} else {
		GLCALL(glDisable(GL_STENCIL_TEST));
	}
}

void Renderer::setStencilMask(int mask) {
	GLCALL(glStencilMask(mask));
}

void Renderer::setStencil(StencilFunc func, int refVal, int mask) {
	GLCALL(glStencilFunc(int(func), refVal, mask));
}

void Renderer::setStencilOp(StencilOp passOp, StencilOp sFailOp, StencilOp dFailOp) {
	GLCALL(glStencilOp(int(sFailOp), int(dFailOp), int(passOp)));
}

void Renderer::setColorMask(bool writteable) {
	GLCALL(glColorMask(writteable, writteable, writteable, writteable));
}

void Renderer::setColorMask(bool r, bool g, bool b, bool a) {
	GLCALL(glColorMask(r, g, b, a));
}

void Renderer::setColorMask(int buffer, bool writteable) {
	GLCALL(glColorMaski(buffer, writteable, writteable, writteable, writteable));
}

void Renderer::setColorMask(int buffer, bool r, bool g, bool b, bool a) {
	GLCALL(glColorMaski(buffer, r, g, b, a));
}

void Renderer::setBlendMode(BlendMode mode)	 {
	if (mode == BlendMode::Enable) {
		GLCALL(glEnable(GL_BLEND));
	} else {
		GLCALL(glDisable(GL_BLEND));
	}
}

void Renderer::setBlendFactor(BlendFactor src, BlendFactor dst) {
	GLCALL(glBlendFunc(int(src), int(dst)));
}

void Renderer::setBlendFactor(int buffer, BlendFactor src, BlendFactor dst) {
	GLCALL(glBlendFunci(buffer, int(src), int(dst)));
}

void Renderer::setBlendFactorSeparate(BlendFactor srcGRB, BlendFactor dstRGB, BlendFactor srcA, BlendFactor dstA) {
	GLCALL(glBlendFuncSeparate(int(srcGRB), int(dstRGB), int(srcA), int(dstA)));
}

void Renderer::setBlendFactorSeparate(int buffer, BlendFactor srcGRB, BlendFactor dstRGB, BlendFactor srcA, BlendFactor dstA) {
	GLCALL(glBlendFuncSeparatei(buffer, int(srcGRB), int(dstRGB), int(srcA), int(dstA)));
}

void Renderer::setBlendFunc(BlendFunc func) {
	GLCALL(glBlendEquation(int(func)));
}

void Renderer::setBlendFunc(int buffer, BlendFunc func) {
	GLCALL(glBlendEquationi(buffer, int(func)));
}

void Renderer::setBlendColor(const glm::vec4& c) {
	GLCALL(glBlendColor(c.r, c.g, c.b, c.a));
}


void Renderer::setGPUPipelineState(const GPUPipelineState& pipelineState) {
	setCullFaceMode(pipelineState.cullMode);
	setFaceWindingOrder(pipelineState.cullFaceWindingOrder);

	setShadeMode(pipelineState.shadeMode);
	setFillMode(pipelineState.fillMode);

	setDepthMode(pipelineState.depthMode);
	setDepthFunc(pipelineState.depthFunc);
	setDepthMask(pipelineState.depthMask > 0 ? true : false);
	setStencilMode(pipelineState.stencilMode);
	setStencilMask(pipelineState.stencilMask);
	setStencilOp(pipelineState.stencilPassOp, pipelineState.stencilFailOp, pipelineState.stencilDepthFailOp);

	setBlendMode(pipelineState.blendMode);
	setBlendFactor(pipelineState.blendSrcFactor, pipelineState.blendDstFactor);
	setBlendFunc(pipelineState.blendFunc);
	setBlendColor({ pipelineState.blendColor[0], pipelineState.blendColor[1], pipelineState.blendColor[2], pipelineState.blendColor[3] });
}


bool Renderer::setupFullScreenQuad() {
	Vertex quadVertices[] = {
		Vertex({-1.f, -1.f, 0.f}, {0.f, 0.f}),
		Vertex({1.f, -1.f, 0.f}, {1.f, 0.f}),
		Vertex({1.f, 1.f, 0.f}, {1.f, 1.f}),
		Vertex({-1.f, 1.f, 0.f}, {0.f, 1.f}),
	};

	unsigned int quadIndices[] = {
		0,1,2,
		2,3,0
	};

	VertexLayoutDescription vertLayoutDesc;
	m_quadVAO.reset(new VertexArray());
	m_quadVBO.reset(new Buffer());
	m_quadIBO.reset(new Buffer());

	m_quadVAO->bind();
	m_quadVBO->bind(Buffer::Target::VertexBuffer);
	m_quadIBO->bind(Buffer::Target::IndexBuffer);
	m_quadVBO->loadData(&quadVertices[0], sizeof(Vertex) * 4, Buffer::Usage::StaticDraw, 4);
	m_quadIBO->loadData(&quadIndices[0], sizeof(unsigned int) * 6, Buffer::Usage::StaticDraw, 6);

	vertLayoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 3, 0);
	vertLayoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 2, 1);

	m_quadVAO->storeVertexLayout(vertLayoutDesc);

	m_quadVAO->unbind();
	m_quadVBO->unbind();
	m_quadIBO->unbind();

	return true;
}


void Renderer::drawFullScreenQuad() {
	RenderTask_t task;
	task.vao = m_quadVAO.get();
	task.indexCount = m_quadIBO->getElementCount();
	task.primitive = PrimitiveType::Triangle;
	m_renderTechnique->performTask(task);
}