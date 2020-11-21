#pragma once
#include"ShadowMapping.h"
#include<memory>


class Renderer;
class FrameBuffer;
class Texture;
class Buffer;

class SpotLightShadowMapping : public IShadowMapping {
public:
	SpotLightShadowMapping(IRenderTechnique* rt, const glm::vec2& shadowMapResolution);
	~SpotLightShadowMapping();

	bool initialize() override;
	void cleanUp() override;
	void beginShadowPhase(const Scene_t& scene, const Light_t& light) override;
	void endShadowPhase() override;
	void beginLighttingPhase(const Light_t& light, ShaderProgram* shader) override;
	void endLighttingPhase(const Light_t& light, ShaderProgram* shader) override;
	void onShadowMapResolutionChange(float w, float h) override;

private:
	Camera_t makeLightCamera(const Light_t& light);

private:
	std::unique_ptr<FrameBuffer> m_shadowMapFBO;
	std::unique_ptr<Texture> m_shadowMap;
	std::unique_ptr<Buffer> m_shadowUBO;
	std::shared_ptr<ShaderProgram> m_shader;

	Camera_t m_lightCamera;
	Viewport_t m_shadowViewport;;
	glm::vec2 m_shadowMapResolution;
};