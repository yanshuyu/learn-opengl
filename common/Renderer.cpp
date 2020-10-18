#include"Renderer.h"
#include"Util.h"
#include"Scene.h"
#include"ShaderProgamMgr.h"
#include"ForwardRenderer.h"
#include"DeferredRenderer.h"
#include"Texture.h"
#include"GuiMgr.h"
#include<glm/gtx/transform.hpp>
#include<functional>


Renderer::Renderer(const RenderingSettings_t& settings, Mode mode) : m_renderingSettings(settings)
, m_renderMode(Mode::None)
, m_renderTechnique(nullptr)
, m_renderContext()
, m_scene(nullptr) {
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


void Renderer::onWindowResize(float w, float h) {
	m_renderingSettings.renderSize = { w, h };
	m_renderTechnique->onWindowResize(w, h);
}


void Renderer::renderScene(Scene* s) {
	m_scene = s;
	m_sceneRenderInfo = s->getSceneRenderInfo();
	
	if (m_clearColor != m_sceneRenderInfo->camera.backgrounColor)
		setClearColor(m_sceneRenderInfo->camera.backgrounColor);
	if (m_viewPort != m_sceneRenderInfo->camera.viewport)
		setViewPort(m_sceneRenderInfo->camera.viewport);

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

void Renderer::pullingRenderTask(std::weak_ptr<ShaderProgram> activeShader) {
	if (m_scene) {
		if (!activeShader.expired())
			m_renderTechnique->setActiveShader(activeShader);

		m_renderContext.clearMatrix();
		m_scene->render(&m_renderContext);
	}
}

void Renderer::setViewPort(const Viewport_t& vp) {
	m_viewPort = vp;
	GLCALL(glViewport(m_viewPort.x, m_viewPort.y, m_viewPort.width, m_viewPort.height));
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
	m_renderTechnique->clearScreen(flags);
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

void Renderer::setDepthTestMode(DepthTestMode mode) {
	switch (mode)
	{
	case DepthTestMode::Enable:
		GLCALL(glEnable(GL_DEPTH_TEST));
		GLCALL(glDepthMask(GL_TRUE));
		break;

	case DepthTestMode::Disable:
		GLCALL(glDisable(GL_DEPTH_TEST));
		break;
	case DepthTestMode::ReadOnly:
		GLCALL(glEnable(GL_DEPTH_TEST));
		GLCALL(glDepthMask(GL_FALSE));
		break;

	default:
		break;
	}
}

void Renderer::setDepthTestFunc(DepthFunc func) {
	GLCALL(glDepthFunc(int(func)));
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


