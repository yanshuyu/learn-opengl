#pragma once
#include<glad/glad.h>
#include"RendererCore.h"
#include"ShaderProgram.h"
#include"RenderTechnique.h"

class Scene;
class Texture;


class Renderer {

#ifdef _DEBUG
public:
	static void visualizeTexture(Texture* tex, const glm::vec2& windowSz, const glm::vec4& rect);
	static void visualizeDepthBuffer(Texture* tex, const glm::vec2& windowSz, const glm::vec4& rect, float near = 0.1f, float far = 1000.f);
#endif // _DEBUG

public:
	enum class Mode {
		None,
		Forward,
		Deferred,
	};

public:
	Renderer(const RenderingSettings_t& settings, Mode mode = Mode::None);
	virtual ~Renderer();

	Renderer(const Renderer& other) = delete;
	Renderer(Renderer&& rv) = delete;
	Renderer& operator = (const Renderer& other) = delete;
	Renderer& operator = (Renderer&& rv) = delete;


	//bool initialize();
	void clenUp();
	bool setRenderMode(Mode mode);
	Mode getRenderMode() const;
	bool isValid() const;
	void onWindowResize(float w, float h);

	void setShadowMapResolution(float w, float h);
	void setClearColor(const glm::vec4& color);
	void setClearDepth(float d);
	void setClearStencil(int m);
	void setViewPort(const Viewport_t& vp);
	void clearScrren(int flags);

	void renderScene(Scene* s);
	void renderTask(const RenderTask_t& task);
	void pullingRenderTask();

	inline  glm::vec4 getClearColor() const {
		return m_renderTechnique->getClearColor();
	}

	inline float getClearDepth() const {
		return m_renderTechnique->getClearDepth();
	}

	inline int getClearStencil() const {
		return m_renderTechnique->getClearStencil();
	}

	inline Viewport_t getViewport() const {
		return m_renderTechnique->getViewport();
	}

private:
	RenderingSettings_t m_renderingSettings;
	Mode m_renderMode;
	RenderContext m_renderContext;
	Scene* m_scene;
	std::unique_ptr<RenderTechnique> m_renderTechnique;
};