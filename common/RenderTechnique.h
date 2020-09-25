#pragma once
#include"RendererCore.h"
#include<string>

//
// uniform block structs
//
struct DirectionalLightBlock {
	glm::vec4 color;
	glm::vec3 inverseDiretion;
};

struct PointLightBlock {
	glm::vec4 position;
	glm::vec4 color;
};

struct SpotLightBlock {
	glm::vec4 position;
	glm::vec4 color;
	glm::vec3 inverseDirection;
	glm::vec2 angles;
};

struct ShadowBlock {
	glm::mat4 lightVP;
	float shadowStrength;
	float depthBias;
	int shadowType;
};



class RenderTechnique {
public:
	enum class RenderPass {
		None,
		DepthPass,
		GeometryPass,
		UnlitPass,
		ShadowPass,
		LightPass,
		TransparencyPass,
	};

public:
	RenderTechnique();
	virtual ~RenderTechnique() {}

	virtual bool intialize() = 0;
	virtual void cleanUp() = 0;

	void setClearColor(const glm::vec4& color);
	void setClearDepth(float d);
	void setClearStencil(int m);
	void setViewPort(const Viewport_t& vp);

	virtual void prepareForSceneRenderInfo(const SceneRenderInfo_t* si) = 0;
	virtual bool shouldVisitScene() const = 0;

	virtual void clearScrren(int flags) = 0;

	virtual void beginFrame() = 0;
	virtual void endFrame() = 0;
	
	virtual void beginDepthPass() = 0;
	virtual void endDepthPass() = 0;

	virtual void beginGeometryPass() = 0;
	virtual void endGeometryPass() = 0;

	virtual void beginUnlitPass() = 0;
	virtual void endUnlitPass() = 0;

	virtual void beginLightPass(const Light_t& l) = 0;
	virtual void endLightPass(const Light_t& l) = 0;

	virtual void beginTransparencyPass() = 0;
	virtual void endTransparencyPass() = 0;

	virtual void beginShadowPass(const Light_t& l) = 0;
	virtual void endShadowPass(const Light_t& l) = 0;

	virtual RenderPass currentRenderPass() const = 0;
	virtual std::string identifier() const = 0;

	virtual void onWindowResize(float w, float h) = 0;
	virtual void onShadowMapResolutionChange(float w, float h) = 0;

	virtual void performTask(const RenderTask_t& task) = 0;


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

protected:
	glm::vec4 m_clearColor;
	float m_clearDepth;
	int m_clearStencil;
	Viewport_t m_viewPort;
};