#pragma once
#include<glad/glad.h>
#include"Util.h"
#include"VertexArray.h"
#include"RendererCore.h"
#include"ShaderProgram.h"
#include"RenderTechnique.h"
#include"FrameAllocator.h"
#include"PostProcessingManager.h"

class Scene;
class Texture;
class Buffer;
class VertexArray;
class RenderTarget;


#define MAX_NUM_OPAQUES 1024
#define MAX_NUM_CUTOUTS 512
#define MAX_NUM_TRANSPARENCES 64
#define MAX_NUM_MAIN_LIGHTS 16
#define MAX_NUM_LIGHTS 1024
#define MAX_NUM_CAMERAS 8
#define MAX_NUM_FILTERS 16

const int MAX_NUM_TOTAL_LIGHTS = MAX_NUM_MAIN_LIGHTS + MAX_NUM_LIGHTS;


class Renderer {
protected:
	template<typename T>
	using FrameVector = std::vector<T, StdFrameAlloc<T>>;

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
	Renderer(const glm::vec2& renderSz, Mode mode = Mode::None);
	virtual ~Renderer();

	Renderer(const Renderer& other) = delete;
	Renderer(Renderer&& rv) = delete;
	Renderer& operator = (const Renderer& other) = delete;
	Renderer& operator = (Renderer&& rv) = delete;

	//
	// life cycle
	//
	bool initialize();
	void cleanUp();
	bool setRenderMode(Mode mode);
	Mode getRenderMode() const;
	bool isValid() const;

	//
	// gpu pipeline state management
	//
	void pushGPUPipelineState(GPUPipelineState* pipeLineState);
	void popGPUPipelineState();
	void clearGPUPiepelineStates();

	void pushRenderTarget(RenderTarget* target);
	void popRenderTarget();
	void clearRenerTargets();

	void pushShaderProgram(ShaderProgram* shader);
	void popShadrProgram();
	inline void clearShaderPrograms();

	ShaderProgram* getActiveShaderProgram() const;

	void pushViewport(Viewport_t* viewport);
	void popViewport();
	void clearViewports();
	const Viewport_t* getActiveViewport() const;

	
	void setClearColor(const glm::vec4& color);
	void setClearDepth(float d);
	void setClearStencil(int m);
	void clearScreen(int flags);

	void setColorMask(bool writteable);
	void setColorMask(bool r, bool g, bool b, bool a);
	void setColorMask(int buffer, bool writteable);
	void setColorMask(int buffer, bool r, bool g, bool b, bool a);

	//
	// render taskes
	//
	void submitCamera(const Camera_t& camera, bool isMain = false);
	void submitLight(const Light_t& light);

	inline void submitOpaqueItem(const MeshRenderItem_t& item) {
		ASSERT(item.vao->isVailde());
		if (m_scene.numOpaqueItems < MAX_NUM_OPAQUES)
			m_opaqueItems[m_scene.numOpaqueItems++] = item;
	}

	inline void submitCutOutItem(const MeshRenderItem_t& item) {
		ASSERT(item.vao->isVailde());
		if (m_scene.numCutOutItems < MAX_NUM_CUTOUTS)
			m_cutOutItems[m_scene.numCutOutItems++] = item;
	}

	inline void submitTransparentItem(const MeshRenderItem_t& item) {
		ASSERT(item.vao->isVailde());
		if (m_scene.numTransparentItems < MAX_NUM_TRANSPARENCES)
			m_transparentItems[m_scene.numTransparentItems++] = item;
	}

	inline void submitPostProcessingFilter(const FilterComponent* filter) {
		if (m_numFilters < MAX_NUM_FILTERS)
			m_filters[m_numFilters++] = filter;
	}

	inline void submitSkyBox(const SkyBox_t& skyBox) {
		m_skyBox = skyBox;
		m_scene.skyBox = m_skyBox ? &m_skyBox : m_scene.skyBox;
	}

	void flush(); // render all submited tasks

	// 
	// dispatch compute/draw command
	//
	inline void dispatchCompute(size_t numGroupX = 1, size_t numGroupY = 1, size_t numGroupZ = 1) {
		GLCALL(glDispatchCompute(numGroupX, numGroupY, numGroupZ));
	}

	void executeDrawCommand(const VertexArray* vao, PrimitiveType pt, size_t numVert, size_t numIndex);
	void flushDrawCommands();
	void drawFullScreenQuad();
	void presentFrame(Texture* frame);

	//
	// events
	//
	void onWindowResize(float w, float h);

	//
	// public setter/getter
	//
	inline  glm::vec4 getClearColor() const {
		return m_clearColor;
	}

	inline float getClearDepth() const {
		return m_clearDepth;
	}

	inline int getClearStencil() const {
		return m_clearStencil;
	}

	inline void setShadowMapResolution(const glm::vec2& dimensions) {
		m_shadowMapResolution = dimensions;
		m_renderTechnique->onShadowMapResolutionChange(dimensions.x, dimensions.y);
	}

	inline glm::vec2 getShadowMapResolution() const {
		return m_shadowMapResolution;
	}

	inline glm::vec2 getRenderSize() const {
		return m_renderSize;
	}

	inline IRenderTechnique* getRenderTechnique() const {
		return m_renderTechnique.get();
	}

protected:
	void setGPUPipelineState(const GPUPipelineState& pipelineState);
	bool setupFullScreenQuad();
	
	inline void setViewPort(const Viewport_t& vp) {
		GLCALL(glViewport(vp.x, vp.y, vp.width, vp.height));
	}

	void resetScene();

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
	Mode m_renderMode;
	std::unique_ptr<IRenderTechnique> m_renderTechnique;

	// gpu pipeline states
	std::stack<GPUPipelineState*> m_pipelineStates;
	std::stack<RenderTarget*> m_renderTargets;
	std::stack<ShaderProgram*> m_shaders;
	std::stack<Viewport_t*> m_viewports;

	glm::vec4 m_clearColor;
	float m_clearDepth;
	int m_clearStencil;

	// screen quad
	std::unique_ptr<VertexArray> m_quadVAO;
	std::unique_ptr<Buffer> m_quadVBO;
	std::unique_ptr<Buffer> m_quadIBO;

	// renderable scene
	std::array<MeshRenderItem_t, MAX_NUM_OPAQUES> m_opaqueItems;
	std::array<MeshRenderItem_t, MAX_NUM_CUTOUTS> m_cutOutItems;
	std::array<MeshRenderItem_t, MAX_NUM_TRANSPARENCES> m_transparentItems;
	std::array<Light_t, MAX_NUM_MAIN_LIGHTS> m_mainLights;
	std::array<Light_t, MAX_NUM_LIGHTS> m_lights;
	std::array<Camera_t, MAX_NUM_CAMERAS> m_cameras;
	std::array<const FilterComponent*, MAX_NUM_FILTERS> m_filters;
	Camera_t* m_mainCamera;
	SkyBox_t m_skyBox;
	Scene_t m_scene;
	size_t m_numFilters;

	//post processing
	PostProcessingManager m_postProcessingMgr;

	// render settings
	glm::vec2 m_renderSize;
	glm::vec2 m_shadowMapResolution;
};