#pragma once
#include"SpotLightShadowMapping.h"
#include<memory>

class Renderer;
class FrameBuffer;
class Texture;

class PointLightShadowMapping : public ShadowMapping {
public:
	PointLightShadowMapping(Renderer* renderer, const glm::vec2& shadowMapResolution);
	~PointLightShadowMapping();

	bool initialize() override;
	void cleanUp() override;
	void beginShadowPhase(const Light_t& light, const Camera_t& camera) override;
	void endShadowPhase(const Light_t& light) override;
	void beginLighttingPhase(const Light_t& light, ShaderProgram* shader) override;
	void endLighttingPhase(const Light_t& light, ShaderProgram* shader) override;
	void onShadowMapResolutionChange(float w, float h) override;

private:
	std::vector<glm::mat4> calclightLightCameraMatrixs(const Light_t& l);

private:
	Renderer* m_renderer;
	Viewport_t m_rendererViewPort;
	glm::vec2 m_shadowMapResolution;

	std::unique_ptr<FrameBuffer> m_FBO;
	std::unique_ptr<Texture> m_cubeShadowMap;
};