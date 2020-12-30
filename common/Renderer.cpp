#include"Renderer.h"
#include"Util.h"
#include"Scene.h"
#include"ShaderProgamMgr.h"
#include"VertexArray.h"
#include"Buffer.h"
#include"Texture.h"
#include"VertexLayoutDescription.h"
#include"ForwardPlusRenderer.h"
#include"GuiMgr.h"
#include<glm/gtx/transform.hpp>
#include<functional>



Renderer::Renderer(const glm::vec2& renderSz, Mode mode) : m_pipelineStates()
, m_renderTargets()
, m_shaders()
, m_viewports()
, m_renderMode(Mode::None)
, m_renderTechnique(nullptr)
, m_clearColor({ 0.f, 0.f, 0.f, 1.f })
, m_clearDepth(1.f)
, m_clearStencil(0.f)
, m_quadVAO(nullptr)
, m_quadVBO(nullptr)
, m_quadIBO(nullptr)
, m_renderSize(renderSz)
, m_shadowMapResolution(1024, 1024)
, _frameAlloc()
, m_opaqueItems(&_frameAlloc)
, m_cutOutItems(&_frameAlloc)
, m_transparentItems(&_frameAlloc)
, m_mainLights(&_frameAlloc)
, m_lights(&_frameAlloc)
, m_cameras(&_frameAlloc)
, m_filters(&_frameAlloc)
, m_mainCamera(nullptr)
, m_skyBox()
, m_scene()
, m_postProcessingMgr(this) {
	if (mode != Mode::None)
		setRenderMode(mode);

	_frameAlloc.markFrame();
	m_opaqueItems.reserve(1024);
	m_transparentItems.reserve(256);
	m_lights.reserve(128);
	m_cameras.reserve(8);
	m_filters.reserve(16);
}

Renderer::~Renderer() {
	cleanUp();
	m_quadVAO.release();
	m_quadVBO.release();
	m_quadIBO.release();
	
	m_postProcessingMgr.cleanUp();

	_frameAlloc.clearAllFrame();
}


bool Renderer::initialize() {
	m_postProcessingMgr.registerStandardFilters();
	return setupFullScreenQuad() && m_postProcessingMgr.initialize();
}


void Renderer::cleanUp() {
	if (m_renderTechnique) {
		m_renderTechnique->cleanUp();
		m_renderTechnique.release();
		m_renderMode = Mode::None;
	}
	
	clearGPUPiepelineStates();
	clearRenerTargets();
	clearShaderPrograms();
	clearViewports();
	m_scene.clear();
	m_mainCamera = nullptr;
}


bool Renderer::setRenderMode(Mode mode) {
	if (m_renderMode != Mode::None)
		return true;

	cleanUp();

	//if (mode == Mode::Forward) {
	//	m_renderTechnique.reset(new ForwardRenderer(this));
	//	if (!m_renderTechnique->intialize()) {
	//		m_renderTechnique.release();
	//		m_renderMode = Mode::None;
	//		return false;
	//	}

	//	m_renderMode = mode;
	//	return true;

	//} else if (mode == Mode::Deferred) {	
	//	m_renderTechnique.reset(new DeferredRenderer(this));
	//	if (!m_renderTechnique->intialize()) {
	//		m_renderTechnique.release();
	//		m_renderMode = mode;
	//		return false;
	//	}
	//	
	//	m_renderMode = mode;
	//	return true;
	//}
	
	m_renderTechnique.reset(new ForwardPlusRenderer(this));
	ASSERT(m_renderTechnique->intialize());

	m_renderMode = mode;

	return true;
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

void Renderer::clearGPUPiepelineStates() {
	while (!m_pipelineStates.empty()) {
		m_pipelineStates.pop();
	}

	setGPUPipelineState(GPUPipelineState::s_defaultState);
}

void Renderer::pushRenderTarget(RenderTarget* target) {
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


void Renderer::clearRenerTargets() {
	while (!m_renderTargets.empty()) {
		m_renderTargets.pop();
	}

	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
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

void Renderer::clearShaderPrograms() {
	while (!m_shaders.empty()) {
		m_shaders.pop();
	}

	GLCALL(glUseProgram(0));
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

void Renderer::clearViewports() {
	while (!m_viewports.empty()) {
		m_viewports.pop();
	}
}

const Viewport_t* Renderer::getActiveViewport() const {
	if (m_viewports.empty() && m_mainCamera)
		return &m_mainCamera->viewport;

	return m_viewports.top();
}

void Renderer::onWindowResize(float w, float h) {
	m_renderSize = glm::vec2(w, h);
	m_renderTechnique->onWindowResize(w, h);
	m_postProcessingMgr.onRenderSizeChange(w, h);
}

void Renderer::updateRenderScene() {
	m_scene.clear();
	m_scene.opaqueItems = m_opaqueItems.data();
	m_scene.numOpaqueItems = m_opaqueItems.size();
	m_scene.cutOutItems = m_cutOutItems.data();
	m_scene.numCutOutItems = m_cutOutItems.size();
	m_scene.transparentItems = m_transparentItems.data();
	m_scene.numTransparentItems = m_transparentItems.size();
	m_scene.mainLights = m_mainLights.data();
	m_scene.numMainLights = m_mainLights.size();
	m_scene.lights = m_lights.data();
	m_scene.numLights = m_lights.size();
	m_scene.cameras = m_cameras.data();
	m_scene.numCameras = m_cameras.size();
	m_scene.mainCamera = m_mainCamera;
	m_scene.skyBox = m_skyBox ? &m_skyBox : nullptr;
}


void Renderer::clearRenderScene() {
	// clen up flushed renderables
	m_opaqueItems.resize(0);
	m_cutOutItems.resize(0);
	m_transparentItems.resize(0);
	m_mainLights.resize(0);
	m_lights.resize(0);
	m_cameras.resize(0);
	m_filters.resize(0);
	m_skyBox.reset();

	// release allocated frame memory, remake a new frame
	_frameAlloc.clearFrame();
	_frameAlloc.markFrame();
}


void Renderer::flush() {
	ASSERT(m_mainCamera);

	clearShaderPrograms();
	clearGPUPiepelineStates();
	clearRenerTargets();
	setDepthMask(true);
	setColorMask(true);
	setStencilMask(0xffffffff);
	clearScreen(ClearFlags::Color | ClearFlags::Depth | ClearFlags::Stencil);
	

	updateRenderScene();
	m_renderTechnique->render(m_scene);
	
	Texture* finalFrame = m_renderTechnique->getRenderedFrame();
	if (m_filters.size() > 0)
		finalFrame = m_postProcessingMgr.applyFilters(finalFrame, m_filters.data(), m_filters.size());

	presentFrame(finalFrame);

	clearRenderScene();
}

void Renderer::presentFrame(Texture* frame) {
	if (!frame)
		return; 

	clearRenerTargets();
	auto shader = ShaderProgramManager::getInstance()->getProgram("FullScreenQuad");
	if (shader.expired())
		shader = ShaderProgramManager::getInstance()->addProgram("FullScreenQuad");
	
	shader.lock()->bind();
	m_quadVAO->bind();

	glActiveTexture(int(Texture::Unit::Defualt));
	frame->bind(Texture::Unit::Defualt);
	shader.lock()->setUniform1("u_color", int(Texture::Unit::Defualt));

	GLCALL(glDrawElements(GL_TRIANGLES, m_quadIBO->getElementCount(), GL_UNSIGNED_INT, 0));
}

void Renderer::drawFullScreenQuad() {
	//MeshRenderItem_t task;
	//task.vao = m_quadVAO.get();
	//task.indexCount = m_quadIBO->getElementCount();
	//task.primitive = PrimitiveType::Triangle;
	//m_renderTechnique->render(task);

	m_quadVAO->bind();
	GLCALL(glDrawElements(GL_TRIANGLES, m_quadIBO->getElementCount(), GL_UNSIGNED_INT, 0));
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


void Renderer::executeDrawCommand(const VertexArray* vao, PrimitiveType pt, size_t numVert, size_t numIndex) {
	vao->bind();
	if (numIndex > 0) {
		GLCALL(glDrawElements(GLenum(pt), numIndex, GL_UNSIGNED_INT, 0));
	}
	else {
		GLCALL(glDrawArrays(GLenum(pt), 0, numVert));
	}
	vao->unbind();
}
