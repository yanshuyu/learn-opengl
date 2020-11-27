#pragma once
#include"RendererCore.h"
#include"Containers.h"
#include<string>
#include<memory>

class ShaderProgram;
class Texture;

class IRenderTechnique {
public:
	IRenderTechnique(Renderer* renderer = nullptr) : m_renderer(renderer) {}
	virtual ~IRenderTechnique() {}

	virtual bool intialize() = 0;
	virtual void cleanUp() = 0;
	virtual std::string identifier() const = 0;

	virtual void render(const MeshRenderItem_t& task) = 0;
	virtual void render(const Scene_t& scene) = 0;
	virtual Texture* getRenderedFrame() = 0;

	virtual void onWindowResize(float w, float h) = 0;
	virtual void onShadowMapResolutionChange(float w, float h) = 0;
	
	inline void setRenderer(Renderer* renderer) {
		m_renderer = renderer;
	}

	inline Renderer* getRenderer() const {
		return m_renderer;
	}

protected:
	Renderer* m_renderer;
};




class RenderTechniqueBase: public IRenderTechnique {
public:
	RenderTechniqueBase(Renderer* renderer = nullptr);
	virtual ~RenderTechniqueBase() {}

protected:
	virtual void render(const Scene_t& scene) override;

	virtual void beginFrame() = 0;
	virtual void endFrame() = 0;
	
	virtual void drawDepthPass(const Scene_t& scene) = 0;
	virtual void drawGeometryPass(const Scene_t& scene) = 0;
	virtual void drawOpaquePass(const Scene_t& scene) = 0;
	virtual void drawTransparentPass(const Scene_t& scene) = 0;

	inline RenderPass getCurrentPass() const {
		return m_pass;
	} 

protected:
	std::shared_ptr<ShaderProgram> m_passShader; // current pass used shader
	RenderPass m_pass;
};