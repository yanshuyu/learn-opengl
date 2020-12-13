#pragma once
#include"ShadowMapping.h"
#include"RenderTarget.h"
#include<memory>


class Renderer;
class Texture;
class Buffer;

class SpotLightShadowMapping : public IShadowMapping {
public:
	SpotLightShadowMapping(IRenderTechnique* rt, const glm::vec2& shadowMapResolution);
	~SpotLightShadowMapping();

	bool initialize() override;
	void cleanUp() override;
	void renderShadow(const Scene_t& scene, const Light_t& light) override;
	void beginRenderLight(const Light_t& light, ShaderProgram* shader) override;
	void endRenderLight(const Light_t& light, ShaderProgram* shader) override;
	void onShadowMapResolutionChange(float w, float h) override;

private:
	Camera_t makeLightCamera(const Light_t& light);

private:
	RenderTarget m_shadowTarget;
	std::unique_ptr<Buffer> m_shadowUBO;
	std::shared_ptr<ShaderProgram> m_shader;

	Camera_t m_lightCamera;
	Viewport_t m_shadowViewport;;
	glm::vec2 m_shadowMapResolution;
};