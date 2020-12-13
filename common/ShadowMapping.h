#pragma once
#include<glm/glm.hpp>
#include"RendererCore.h"

class ShaderProgram;
class IRenderTechnique;

class IShadowMapping {
public:
	IShadowMapping(IRenderTechnique* rt = nullptr):m_renderTech(rt) {}
	virtual ~IShadowMapping() {}

	virtual bool initialize() = 0;
	virtual	void cleanUp() {};
	virtual void renderShadow(const Scene_t& scene, const Light_t& light) = 0;
	virtual void beginRenderLight(const Light_t& light, ShaderProgram* shader) = 0;
	virtual void endRenderLight(const Light_t& light, ShaderProgram* shader) = 0;
	virtual void onShadowMapResolutionChange(float w, float h) = 0;

	inline IRenderTechnique* getRenderTechnique() const {
		return m_renderTech;
	}

	inline void setRendererTechnique(IRenderTechnique* rt) {
		m_renderTech = rt;
	}

protected:
	IRenderTechnique* m_renderTech;
};