#pragma once
#include"ShadowMapping.h"
#include<memory>

//
// cascade shadow mapping (CSM) implementation
//

class RenderTechnique;
class FrameBuffer;
class Texture;
class Buffer;

class DirectionalLightShadowMapping : ShadowMapping {
public:
	DirectionalLightShadowMapping(RenderTechnique* renderer, const glm::vec2& shadowMapResolution, const std::array<float, 2>& cascadeSplitPercentage = {0.2f, 0.5f});;
	bool initialize() override;
	void cleanUp() override;
	void beginShadowPhase(const Light_t& light, const Camera_t& camera) override;
	void endShadowPhase(const Light_t& light, const Camera_t& camera) override;
	void beginLighttingPhase(const Light_t& light, ShaderProgram* shader) override;
	void endLighttingPhase(const Light_t& light, ShaderProgram* shader) override;
	void onShadowMapResolutionChange(float w, float h) override;

#ifdef _DEBUG
	void visualizeShadowMaps(const glm::vec2& wndSize);
#endif // _DEBUG


private:
	void calcViewFrumstumCascades(const Light_t& light, const Camera_t& camera);

private:
	RenderTechnique* m_renderer;
	std::unique_ptr<FrameBuffer> m_FBO;
	std::vector<std::unique_ptr<Texture>> m_shadowMaps;
	glm::vec2 m_shadowMapResolution;
	Viewport_t m_rendererViewPort;

	std::vector<ViewFrustum_t> m_cascades;
	std::vector<float> m_cascadeSplitPercents;
	std::vector<float> m_cascadeFarProjZ;
	std::vector<Camera_t> m_cascadeCameras;
};