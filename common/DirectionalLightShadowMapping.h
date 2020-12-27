#pragma once
#include"ShadowMapping.h"
#include"RenderTarget.h"
#include<memory>

//
// cascade shadow mapping (CSM) implementation
//

class FrameBuffer;
class Texture;
class Buffer;

class DirectionalLightShadowMapping : public IShadowMapping {
public:
	DirectionalLightShadowMapping(IRenderTechnique* rt, const glm::vec2& shadowMapResolution, int numCascade);
	
	static const int s_maxNumCascades;
	
	bool initialize() override;
	void cleanUp() override;
	void renderShadow(const Scene_t& scene, const Light_t& light) override;
	void beginRenderLight(const Light_t& light, ShaderProgram* shader) override;
	void endRenderLight(const Light_t& light, ShaderProgram* shader) override;
	void onShadowMapResolutionChange(float w, float h) override;

#ifdef _DEBUG
	void visualizeShadowMaps(const glm::vec2& wndSize);
#endif // _DEBUG


private:
	void calcViewFrumstumCascades(const Light_t& light, const Camera_t& camera);
	void calcViewFrumstumSplitPercents(const Camera_t& camera, float t);

private:
	RenderTarget m_shadowTarget;
	std::shared_ptr<ShaderProgram> m_shader;

	glm::vec2 m_shadowMapResolution;
	Viewport_t m_shadowViewport;

	int m_numCascade;
	std::vector<float> m_splitPercents;
	std::vector<float> m_cascadeFarProjZ;
	std::vector<Camera_t> m_cascadeCameras;
};