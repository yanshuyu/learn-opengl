#pragma once
#include"RenderTechnique.h"
#include"RenderTaskExecutor.h"
#include<memory>
#include<unordered_map>


class VertexArray;
class Buffer;
class FrameBuffer;
class Texture;
class ShaderProgram;
class SpotLightShadowMapping;
class DirectionalLightShadowMapping;
class PointLightShadowMapping;


class DeferredRenderer : public RenderTechnique {
	friend class DepthPassRenderTaskExecutor;
	friend class GeometryPassRenderTaskExecutor;
	friend class UlitPassRenderTaskExecutror;
	friend class ShadowPassRenderTaskExecutor;
	
	friend class SpotLightShadowMapping;

public:
	DeferredRenderer(Renderer* inkover, const RenderingSettings_t& settings);
	~DeferredRenderer();

	DeferredRenderer(const DeferredRenderer& other) = delete;
	DeferredRenderer(DeferredRenderer&& rv) = delete;
	DeferredRenderer& operator = (const DeferredRenderer& other) = delete;
	DeferredRenderer& operator = (DeferredRenderer&& rv) = delete;

	bool intialize() override;
	void cleanUp() override;

	static const std::string s_identifier;

	void beginFrame() override;
	void endFrame() override;

	void beginDepthPass() override;;
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

	void onWindowResize(float w, float h) override;
	void onShadowMapResolutionChange(float w, float h) override;

	void performTask(const RenderTask_t& task)  override;
	bool shouldRunPass(RenderPass pass) override;

	RenderPass currentRenderPass() const override {
		return m_currentPass;
	}

	std::string identifier() const override {
		return s_identifier;
	}

private:
	bool setupGBuffers();
	//bool setupShadowMap();

private:
	RenderPass m_currentPass;
	std::unordered_map<RenderPass, std::unique_ptr<RenderTaskExecutor>> m_taskExecutors;

	GPUPipelineState m_depthPassPipelineState;
	GPUPipelineState m_geometryPassPipelineState;
	GPUPipelineState m_shadowPassPipelineState;
	GPUPipelineState m_lightPassPipelineState;
	GPUPipelineState m_unlitPassPipelineState;

	// geometry buffers
	std::unique_ptr<FrameBuffer> m_gBuffersFBO;
	std::unique_ptr<Texture> m_posWBuffer;
	std::unique_ptr<Texture> m_normalWBuffer;
	std::unique_ptr<Texture> m_diffuseBuffer;
	std::unique_ptr<Texture> m_specularBuffer;
	std::unique_ptr<Texture> m_emissiveBuffer;
	std::unique_ptr<Texture> m_depthStencilBuffer;

	// light ubo
	std::unique_ptr<Buffer> m_directionalLightUBO;
	std::unique_ptr<Buffer> m_pointLightUBO;
	std::unique_ptr<Buffer> m_spotLightUBO;

	// shadow mapping
	std::unique_ptr<SpotLightShadowMapping> m_spotLightShadow;
	std::unique_ptr<DirectionalLightShadowMapping> m_dirLightShadow;
	std::unique_ptr<PointLightShadowMapping> m_pointLightShadow;
};



