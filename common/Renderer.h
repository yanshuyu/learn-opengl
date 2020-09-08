#pragma once
#include<glad/glad.h>
#include"RendererCore.h"
#include"ShaderProgram.h"


class Scene;


class Renderer {
	friend class Scene;

public:
	Renderer();
	virtual ~Renderer() {}

	Renderer(const Renderer& other) = delete;
	Renderer(Renderer&& rv) = delete;
	Renderer& operator = (const Renderer& other) = delete;
	Renderer& operator = (Renderer&& rv) = delete;

	void setClearColor(float r, float g, float b, float a);
	void setClearDepth(float d);
	void setClearMask(int m);
	void setViewPort(int x, int y, int width, int height);
	void clearScrren(int flags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	virtual void renderScene(Scene* s) = 0;
	virtual void subsimtTask(const RenderTask_t& task) = 0;

	inline  const float* const getClearColor() const {
		return &m_clearColor[0];
	}

	inline float getClearDepth() const {
		return m_clearDepth;
	}

	inline int getClearMask() const {
		return m_clearMask;
	}


protected:
	virtual void beginDepthPass();
	virtual void endDepthPass();

	virtual void beginUnlitPass() = 0;
	virtual void endUnlitPass() = 0;

	virtual void beginLightpass(const Light_t& light) = 0;
	virtual void endLightPass() = 0;

	ShaderProgram* getActiveShaderProgram() const {
		return m_activeShaderProgram.get();
	}
private:
	float m_clearColor[4];
	float m_clearDepth;
	int m_clearMask;

	std::shared_ptr<ShaderProgram> m_activeShaderProgram;
};