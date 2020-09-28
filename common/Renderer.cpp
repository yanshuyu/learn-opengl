#include"Renderer.h"
#include"Util.h"
#include"Scene.h"
#include"ShaderProgamMgr.h"
#include"ForwardRenderer.h"
#include"DeferredRenderer.h"
#include"Texture.h"
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
	auto sri = s->gatherSceneRenderInfo();
	m_renderTechnique->prepareForSceneRenderInfo(sri);
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
	if (sri->lights.empty()) {
		if (m_renderTechnique->shouldRunPass(RenderPass::UnlitPass)) {
			m_renderContext.clearMatrix();
			m_renderTechnique->beginUnlitPass();
			m_renderTechnique->endUnlitPass();
		}
	} else {
		// light passes
		if (m_renderTechnique->shouldRunPass(RenderPass::LightPass)) {
			for (const auto& l : sri->lights) {
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


#ifdef _DEBUG

static void drawQuad(ShaderProgram* shader, Texture* tex, const glm::vec2& windowSz, const glm::vec4& rect, std::function<void(ShaderProgram*)> uniformSetter) {
	static unsigned int vbo = 0;
	static unsigned int ibo = 0;
	static unsigned int vao = 0;

	// generate buffers
	if (!vao) {
		float vertices[] = {
			0.f, 0.f, 0.f, 0.f,
			1.f, 0.f, 1.f, 0.f,
			1.f, 1.f, 1.f, 1.f,
			0.f, 1.f, 0.f, 1.f,
		};

		unsigned int indices[] = {
			0, 1, 2,
			2, 3, 0,
		};

		GLCALL(glGenVertexArrays(1, &vao));
		GLCALL(glGenBuffers(1, &vbo));
		GLCALL(glGenBuffers(1, &ibo));

		GLCALL(glBindVertexArray(vao));
		GLCALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
		GLCALL(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, vertices, GL_STATIC_DRAW));

		GLCALL(glEnableVertexAttribArray(0));
		GLCALL(glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 4, reinterpret_cast<void*>(0)));
		GLCALL(glEnableVertexAttribArray(1));
		GLCALL(glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 4, reinterpret_cast<void*>(sizeof(float) * 2)));

		GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
		GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, indices, GL_STATIC_DRAW));

		GLCALL(glBindVertexArray(0));
		GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	}

	shader->bind();
	GLCALL(glBindVertexArray(vao));

	if (shader->hasUniform("u_MVP")) {
		glm::mat4 m(1.f);
		m = glm::translate(m, { rect.x, rect.y, 0.f });
		m = glm::scale(m, { rect.z, rect.w, 1.f });
		glm::mat4 p = glm::ortho(0.f, windowSz.x, 0.f, windowSz.y, 0.f, 1.f);
		m = p * m;
		shader->setUniformMat4v("u_MVP", &m[0][0]);
	}

	uniformSetter(shader);

	GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, reinterpret_cast<void*>(0)));

	shader->unbind();
	tex->unbind();
	GLCALL(glBindVertexArray(0));
}


void Renderer::visualizeTexture(Texture* tex, const glm::vec2& windowSz, const glm::vec4& rect) {
	auto textureViewerShader = ShaderProgramManager::getInstance()->getProgram("TextureDebugViewer");
	if (!textureViewerShader)
		textureViewerShader = ShaderProgramManager::getInstance()->addProgram("res/shader/TextureDebugViewer.shader");

	ASSERT(textureViewerShader);
	
	drawQuad(textureViewerShader.get(), tex, windowSz, rect, [&](ShaderProgram* shader) {
		if (shader->hasUniform("u_texture")) {
			tex->bind(Texture::Unit::Defualt);
			shader->setUniform1("u_texture", int(Texture::Unit::Defualt));
		}
	});
}

void Renderer::visualizeDepthBuffer(Texture* tex, const glm::vec2& windowSz, const glm::vec4& rect, float near, float far) {
	auto depthViwerShader = ShaderProgramManager::getInstance()->getProgram("DepthBufferDebugViewer");
	if (!depthViwerShader)
		depthViwerShader = ShaderProgramManager::getInstance()->addProgram("res/shader/DepthBufferDebugViewer.shader");

	ASSERT(depthViwerShader);

	drawQuad(depthViwerShader.get(), tex, windowSz, rect, [&](ShaderProgram* shader) {
		if (shader->hasUniform("u_depthTexture")) {
			tex->bind(Texture::Unit::Defualt);
			shader->setUniform1("u_depthTexture", int(Texture::Unit::Defualt));
		}

		if (shader->hasUniform("u_near"))
			shader->setUniform1("u_near", near);

		if (shader->hasUniform("u_far"))
			shader->setUniform1("u_far", far);
	});
}

#endif // _DEBUG

