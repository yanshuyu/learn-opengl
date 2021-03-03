#pragma once
#include"SpotLightShadowMapping.h"
#include"RenderTarget.h"
#include<memory>

class Renderer;
class Texture;

class PointLightShadowMapping : public IShadowMapping {
public:
	PointLightShadowMapping(IRenderTechnique* rt, const glm::vec2& shadowMapResolution);
	~PointLightShadowMapping();

	inline bool initialize() override {
		return setupShadowRenderTarget();
	}

	inline  void cleanUp() override {
		m_shadowTarget.release();
	}

	void renderShadow(const Scene_t& scene, const Light_t& light) override;
	void beginRenderLight(const Light_t& light, ShaderProgram* shader) override;
	void endRenderLight(const Light_t& light, ShaderProgram* shader) override;
	void onShadowMapResolutionChange(float w, float h) override;

private:
	bool setupShadowRenderTarget();
	std::vector<glm::mat4> calclightLightCameraMatrixs(const Light_t& l);

private:
	glm::vec2 m_shadowMapResolution;
	Viewport_t m_shadowViewport;

	std::unique_ptr<RenderTarget> m_shadowTarget;
	std::shared_ptr<ShaderProgram> m_shader;
};