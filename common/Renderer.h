#pragma once
#include<glad/glad.h>
#include"RendererCore.h"
#include"ShaderProgram.h"
#include"RenderTechnique.h"

class Scene;


class Renderer {
public:
	Renderer(RenderTechnique* rt);
	Renderer(std::unique_ptr<RenderTechnique>&& rt);
	virtual ~Renderer() {}

	Renderer(const Renderer& other) = delete;
	Renderer(Renderer&& rv) = delete;
	Renderer& operator = (const Renderer& other) = delete;
	Renderer& operator = (Renderer&& rv) = delete;

	void setClearColor(const glm::vec4& color);
	void setClearDepth(float d);
	void setClearStencil(int m);
	void setViewPort(const Viewport_t& vp);
	void clearScrren(int flags);

	void setRenderTechnique(RenderTechnique* rt);
	void setRenderTechnique(std::unique_ptr<RenderTechnique>&& rt);

	bool initialize();
	void clenUp();
	void renderScene(Scene* s);
	void subsimtTask(const RenderTask_t& task);

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

	inline RenderTechnique* getRenderTechnique() const {
		return m_renderTechnique.get();
	}

private:
	RenderContext m_renderContext;
	std::unique_ptr<RenderTechnique> m_renderTechnique;
};