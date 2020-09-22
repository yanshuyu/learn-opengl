#include"Renderer.h"
#include"Util.h"
#include"Scene.h"
#include"ShaderProgamMgr.h"


Renderer::Renderer(RenderTechnique* rt): Renderer(std::unique_ptr<RenderTechnique>(rt)) {

}

Renderer::Renderer(std::unique_ptr<RenderTechnique>&& rt) : m_renderTechnique(std::move(rt))
, m_renderContext() {
	m_renderContext.setRenderer(this);
}


bool Renderer::initialize() {
	return m_renderTechnique->intialize();
}


void Renderer::clenUp() {
	m_renderTechnique->cleanUp();
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


void Renderer::setRenderTechnique(RenderTechnique* rt) {
	m_renderTechnique.reset(rt);
}


void Renderer::setRenderTechnique(std::unique_ptr<RenderTechnique>&& rt) {
	m_renderTechnique = std::move(rt);
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

