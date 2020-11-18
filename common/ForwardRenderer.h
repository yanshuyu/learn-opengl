#pragma once
#include"RenderTechnique.h"
#include"RenderTaskExecutor.h"
#include<unordered_map>
#include<memory>


class Buffer;
class FrameBuffer;
class Texture;
class SpotLightShadowMapping;
class DirectionalLightShadowMapping;
class PointLightShadowMapping;


class ForwardRenderer : public RenderTechnique {
	friend class DepthPassRenderTaskExecutor;
	friend class UlitPassRenderTaskExecutror;
	friend class LightPassRenderTaskExecuter;
	friend class ShadowPassRenderTaskExecutor;

	friend class SpotLightShadowMapping;

public:
	ForwardRenderer(Renderer* renderer, const RenderingSettings_t& settings);
	~ForwardRenderer();

	ForwardRenderer(const ForwardRenderer& other) = delete;
	ForwardRenderer(ForwardRenderer&& rv) = delete;
	ForwardRenderer& operator = (const ForwardRenderer& other) = delete;
	ForwardRenderer& operator = (ForwardRenderer&& rv) = delete;

	static const std::string s_identifier;

	void clearScreen(int flags) override;

	bool intialize() override;
	void cleanUp() override;

	void beginFrame() override;
	void endFrame() override;

	void beginDepthPass() override;
	void endDepthPass() override;

	void beginGeometryPass() override;
	void endGeometryPass() override;

	void beginUnlitPass() override;
	void endUnlitPass() override;


	void beginShadowPass(const Light_t& l) override;
	void endShadowPass(const Light_t& l) override;

	void beginLightPass(const Light_t& l) override;
	void endLightPass(const Light_t& l) override;

	void beginTransparencyPass() override;
	void endTransparencyPass() override;

	void performTask(const RenderTask_t& task) override;
	bool shouldRunPass(RenderPass pass) override;

	void onWindowResize(float w, float h) override;
	void onShadowMapResolutionChange(float w, float h);

	RenderPass currentRenderPass() const override {
		return m_currentPass;
	}

	std::string identifier() const override {
		return s_identifier;
	}

private:
	RenderPass m_currentPass;
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
	std::unique_ptr<SpotLightShadowMapping> m_spotLightShadow;
	std::unique_ptr<DirectionalLightShadowMapping> m_dirLightShadow;
	std::unique_ptr<PointLightShadowMapping> m_pointLightShadow;

};