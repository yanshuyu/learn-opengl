#pragma once
#include<glad/glad.h>
#include"RendererCore.h"
#include"ShaderProgram.h"
#include"RenderTechnique.h"

class Scene;
class Texture;
class FrameBuffer;
class Buffer;
class VertexArray;


class Renderer {
protected:
	struct Vertex {
		glm::vec3 position;
		glm::vec2 uv;

		Vertex(const glm::vec3& _position, const glm::vec2& _uv) {
			position = _position;
			uv = _uv;
		}
	};

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

	void pushGPUPipelineState(GPUPipelineState* pipeLineState);
	void popGPUPipelineState();

	void pushRenderTarget(FrameBuffer* target);
	void popRenderTarget();

	bool initialize();
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

	void setColorMask(bool writteable);
	void setColorMask(bool r, bool g, bool b, bool a);
	void setColorMask(int buffer, bool writteable);
	void setColorMask(int buffer, bool r, bool g, bool b, bool a);

	void drawFullScreenQuad();
	void renderScene(Scene* s);
	void renderTask(const RenderTask_t& task);
	void pullingRenderTask(std::weak_ptr<ShaderProgram> activeShader = std::weak_ptr<ShaderProgram>());


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

protected:
	void setGPUPipelineState(const GPUPipelineState& pipelineState);
	bool setupFullScreenQuad();

	// clipping states
	void setCullFaceMode(CullFaceMode mode);
	void setFaceWindingOrder(FaceWindingOrder order);

	// rasterization states
	void setShadeMode(ShadeMode mode);
	void setFillMode(FillMode mode);

	// depth/stencil states
	void setDepthMode(DepthMode mode);
	void setDepthFunc(DepthFunc func);
	void setDepthMask(bool writable);
	void setStencilMode(StencilMode mode);
	void setStencilMask(int mask);
	void setStencil(StencilFunc func, int refVal, int mask = 0xffffffff);
	void setStencilOp(StencilOp passOp, StencilOp sFailOp, StencilOp dFailOp);

	// blend state
	void setBlendMode(BlendMode mode);
	void setBlendFactor(BlendFactor src, BlendFactor dst);
	void setBlendFactor(int buffer, BlendFactor src, BlendFactor dst);
	void setBlendFactorSeparate(BlendFactor srcGRB, BlendFactor dstRGB, BlendFactor srcA, BlendFactor dstA);
	void setBlendFactorSeparate(int buffer, BlendFactor srcGRB, BlendFactor dstRGB, BlendFactor srcA, BlendFactor dstA);
	void setBlendFunc(BlendFunc func);
	void setBlendFunc(int buffer, BlendFunc func);
	void setBlendColor(const glm::vec4& c);

protected:
	std::stack<GPUPipelineState*> m_pipelineStates;
	std::stack<FrameBuffer*> m_renderTargets;

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

	// screen quad
	std::unique_ptr<VertexArray> m_quadVAO;
	std::unique_ptr<Buffer> m_quadVBO;
	std::unique_ptr<Buffer> m_quadIBO;
};