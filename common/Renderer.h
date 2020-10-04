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
	void clearScreen(int flags);

	void setCullFaceMode(CullFaceMode mode);
	void setFaceWindingOrder(FaceWindingOrder order);

	void setDepthTestMode(DepthTestMode mode);
	void setDepthTestFunc(DepthFunc func);

	void setColorMask(bool writteable);
	void setColorMask(bool r, bool g, bool b, bool a);
	void setColorMask(int buffer, bool writteable);
	void setColorMask(int buffer, bool r, bool g, bool b, bool a);

	void setBlendMode(BlendMode mode);
	void setBlendFactor(BlendFactor src, BlendFactor dst);
	void setBlendFactor(int buffer, BlendFactor src, BlendFactor dst);
	void setBlendFactorSeparate(BlendFactor srcGRB, BlendFactor dstRGB, BlendFactor srcA, BlendFactor dstA);
	void setBlendFactorSeparate(int buffer, BlendFactor srcGRB, BlendFactor dstRGB, BlendFactor srcA, BlendFactor dstA);
	void setBlendFunc(BlendFunc func);
	void setBlendFunc(int buffer, BlendFunc func);
	void setBlendColor(const glm::vec4& c);

	void renderScene(Scene* s);
	void renderTask(const RenderTask_t& task);
	void pullingRenderTask(ShaderProgram* activeShader = nullptr);


	inline  glm::vec4 getClearColor() const {
		return m_clearColor;
	}

	inline float getClearDepth() const {
		return m_clearDepth;
	}

	inline int getClearStencil() const {
		return m_clearStencil;
	}

	inline Viewport_t getViewport() const {
		return m_viewPort;
	}

	inline const RenderingSettings_t* getRenderingSettings() const {
		return &m_renderingSettings;
	}

	inline const SceneRenderInfo_t* getSceneRenderInfo() const {
		return m_sceneRenderInfo;
	}

private:
	RenderingSettings_t m_renderingSettings;
	Mode m_renderMode;
	RenderContext m_renderContext;
	Scene* m_scene;
	SceneRenderInfo_t* m_sceneRenderInfo;
	std::unique_ptr<RenderTechnique> m_renderTechnique;

	glm::vec4 m_clearColor;
	float m_clearDepth;
	int m_clearStencil;
	Viewport_t m_viewPort;
};