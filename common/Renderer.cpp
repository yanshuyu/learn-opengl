#include"Renderer.h"
#include"Util.h"
#include"Scene.h"
#include"ShaderProgamMgr.h"
#include"ForwardRenderer.h"
#include"DeferredRenderer.h"


Renderer::Renderer(float w, float h, Mode mode) : m_wndWidth(w)
, m_wndHeight(h)
, m_renderMode(Mode::None)
, m_renderTechnique(nullptr)
, m_renderContext() {
	if (mode != Mode::None)
		setRenderMode(mode);
	m_renderContext.setRenderer(this);
}

Renderer::~Renderer() {
	clenUp();
}


//bool Renderer::initialize() {
//	return __setRenderMode(m_renderMode);
//}


void Renderer::clenUp() {
	if (m_renderTechnique) {
		m_renderTechnique->cleanUp();
		m_renderTechnique.release();
		m_renderMode = Mode::None;
	}
}


bool Renderer::setRenderMode(Mode mode) {
	if (m_renderMode == mode)
		return true;

	clenUp();

	if (mode == Mode::Forward) {
		m_renderTechnique.reset(new ForwardRenderer());
		if (!m_renderTechnique->intialize()) {
			m_renderTechnique.release();
			m_renderMode = Mode::None;
			return false;
		}

		m_renderMode = mode;
		return true;

	} else if (mode == Mode::Deferred) {	
		m_renderTechnique.reset(new DeferredRenderer(m_wndWidth, m_wndHeight));
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


Renderer::Mode Renderer::getRenderMode() const {
	return m_renderMode;
}


bool Renderer::isValid() const {
	if (m_renderMode == Mode::None)
		return false;

	return m_renderTechnique != nullptr;
}


void Renderer::onWindowResize(float w, float h) {
	m_wndWidth = w;
	m_wndHeight = h;
	m_renderTechnique->onWindowResize(w, h);
}


void Renderer::renderScene(Scene* s) {
	auto sri = s->gatherSceneRenderInfo();
	m_renderTechnique->prepareForSceneRenderInfo(sri);
	m_renderTechnique->beginFrame();

	// pre-z pass
	m_renderContext.clearMatrix();
	m_renderTechnique->beginDepthPass();
	if(m_renderTechnique->shouldVisitScene()) s->render(&m_renderContext);
	m_renderTechnique->endDepthPass();

	// G pass
	m_renderContext.clearMatrix();
	m_renderTechnique->beginGeometryPass();
	if (m_renderTechnique->shouldVisitScene()) s->render(&m_renderContext);
	m_renderTechnique->endGeometryPass();

	// ulit pass
	if (sri->lights.empty()) {
		m_renderContext.clearMatrix();
		m_renderTechnique->beginUnlitPass();
		if (m_renderTechnique->shouldVisitScene()) s->render(&m_renderContext);
		m_renderTechnique->endUnlitPass();

	} else {
		// light passes
		for (const auto& l : sri->lights) {
			m_renderContext.clearMatrix();
			m_renderTechnique->beginLightPass(l);
			if (m_renderTechnique->shouldVisitScene()) s->render(&m_renderContext);
			m_renderTechnique->endLightPass(l);
		}
	}

	// transparency pass

	m_renderTechnique->endFrame();
}

void Renderer::subsimtTask(const RenderTask_t& task) {
	m_renderTechnique->performTask(task);
}


void Renderer::setViewPort(const Viewport_t& vp) {
	m_renderTechnique->setViewPort(vp);
}


void Renderer::setClearColor(const glm::vec4& color) {
	m_renderTechnique->setClearColor(color);
}


void Renderer::setClearDepth(float d) {
	m_renderTechnique->setClearDepth(d);
}


void Renderer::setClearStencil(int m) {
	m_renderTechnique->setClearStencil(m);
}


void Renderer::clearScrren(int flags) {
	m_renderTechnique->clearScrren(flags);
}

