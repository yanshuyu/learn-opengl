#pragma once
#include"RendererCore.h"
#include<string>


class ShaderProgram;

class RenderTechnique {
public:
	RenderTechnique(Renderer* invoker);
	virtual ~RenderTechnique() {}

	virtual bool intialize() = 0;
	virtual void cleanUp() = 0;

	virtual void clearScreen(int flags) = 0;

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
	virtual bool shouldRunPass(RenderPass pass) = 0;

	void setActiveShader(ShaderProgram* shader) {
		m_activeShader = shader;
	}

	ShaderProgram* getActiveShader(ShaderProgram* shader) const {
		return m_activeShader;
	}

protected:
	Renderer* m_invoker;
	ShaderProgram* m_activeShader;
};