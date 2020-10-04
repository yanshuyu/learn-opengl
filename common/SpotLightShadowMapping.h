#pragma once
#include"ShadowMapping.h"
#include<memory>


class Renderer;
class FrameBuffer;
class Texture;
class Buffer;

class SpotLightShadowMapping : public ShadowMapping {
public:
	SpotLightShadowMapping(Renderer* renderer, const glm::vec2& shadowMapResolution);
	~SpotLightShadowMapping();

	bool initialize() override;
	void cleanUp() override;
	void beginShadowPhase(const Light_t& light, const Camera_t& camera) override;
	void endShadowPhase(const Light_t& light, const Camera_t& camera) override;
	void beginLighttingPhase(const Light_t& light, ShaderProgram* shader) override;
	void endLighttingPhase(const Light_t& light, ShaderProgram* shader) override;
	void onShadowMapResolutionChange(float w, float h) override;

#ifdef _DEBUG
	void visualizeShadowMap(const glm::vec2& wndSz, const glm::vec4& rect);
#endif // _DEBUG


private:
	Camera_t makeLightCamera(const Light_t& light);

private:
	Renderer* m_renderer;
	std::unique_ptr<FrameBuffer> m_shadowMapFBO;
	std::unique_ptr<Texture> m_shadowMap;
	std::unique_ptr<Buffer> m_shadowUBO;
	Camera_t m_lightCamera;
	Viewport_t m_rendererViewPort;
	glm::vec2 m_shadowMapResolution;
};