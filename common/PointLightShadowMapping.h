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

	bool initialize() override;
	void cleanUp() override;
	void renderShadow(const Scene_t& scene, const Light_t& light) override;
	void beginRenderLight(const Light_t& light, ShaderProgram* shader) override;
	void endRenderLight(const Light_t& light, ShaderProgram* shader) override;
	void onShadowMapResolutionChange(float w, float h) override;

private:
	std::vector<glm::mat4> calclightLightCameraMatrixs(const Light_t& l);

private:
	glm::vec2 m_shadowMapResolution;
	Viewport_t m_shadowViewport;

	RenderTarget m_shadowTarget;
	std::shared_ptr<ShaderProgram> m_shader;
};