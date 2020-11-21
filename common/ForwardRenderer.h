#pragma once
#include"RenderTechnique.h"
#include"RenderTaskExecutor.h"
#include<unordered_map>
#include<memory>


class Buffer;
class FrameBuffer;
class Texture;
class IShadowMapping;


class ForwardRenderer : public RenderTechniqueBase {
	friend class DepthPassRenderTaskExecutor;
	friend class UlitPassRenderTaskExecutror;
	friend class LightPassRenderTaskExecuter;
	friend class ShadowPassRenderTaskExecutor;
	friend class SpotLightShadowMapping;

public:
	ForwardRenderer(Renderer* renderer);
	~ForwardRenderer();

	ForwardRenderer(const ForwardRenderer& other) = delete;
	ForwardRenderer(ForwardRenderer&& rv) = delete;
	ForwardRenderer& operator = (const ForwardRenderer& other) = delete;
	ForwardRenderer& operator = (ForwardRenderer&& rv) = delete;

	static const std::string s_identifier;

	bool intialize() override;
	void cleanUp() override;

	void beginFrame() override;
	void endFrame() override;

	void render(const MeshRenderItem_t& task) override;

	void drawDepthPass(const Scene_t& scene) override;
	void drawGeometryPass(const Scene_t& scene) override;
	void drawOpaquePass(const Scene_t& scene) override;
	void drawTransparentPass(const Scene_t& scene) override {};

	void onWindowResize(float w, float h) override;
	void onShadowMapResolutionChange(float w, float h) override;

	inline std::string identifier() const override {
		return s_identifier;
	}

protected:
	void drawUnlitScene(const Scene_t& scene);
	void drawLightScene(const Scene_t& scene, const Light_t& light);
	void drawLightShadow(const Scene_t& scene, const Light_t& light);

protected:
	std::unordered_map<RenderPass, std::unique_ptr<RenderTaskExecutor>> m_taskExecutors;
	
	GPUPipelineState m_depthPassPipelineState;
	GPUPipelineState m_shadowPassPipelineState;
	GPUPipelineState m_lightPassPipelineState;
	GPUPipelineState m_unlitPassPipelineState;

	// light uniform blocks
	std::unique_ptr<Buffer> m_directionalLightUBO;
	std::unique_ptr<Buffer> m_pointLightUBO;
	std::unique_ptr<Buffer> m_spotLightUBO;
	
	// shadow mapping
	std::unordered_map<LightType, std::unique_ptr<IShadowMapping>> m_shadowMappings;

};