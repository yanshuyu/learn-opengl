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
class IShadowMapping;


class DeferredRenderer : public RenderTechniqueBase {
	friend class DepthPassRenderTaskExecutor;
	friend class GeometryPassRenderTaskExecutor;
	friend class UlitPassRenderTaskExecutror;
	friend class ShadowPassRenderTaskExecutor;
	
	friend class SpotLightShadowMapping;

public:
	DeferredRenderer(Renderer* renderer);
	~DeferredRenderer();

	DeferredRenderer(const DeferredRenderer& other) = delete;
	DeferredRenderer(DeferredRenderer&& rv) = delete;
	DeferredRenderer& operator = (const DeferredRenderer& other) = delete;
	DeferredRenderer& operator = (DeferredRenderer&& rv) = delete;

	bool intialize() override;
	void cleanUp() override;

	static const std::string s_identifier;

	void render(const MeshRenderItem_t& task) override;

	void beginFrame() override;
	void endFrame() override;

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
	bool setupGBuffers();
	void drawUnlitScene(const Scene_t& scene);
	void drawLightScene(const Scene_t& scene, const Light_t& light);
	void drawLightShadow(const Scene_t& scene, const Light_t& light);

private:
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
	std::unordered_map<LightType, std::unique_ptr<IShadowMapping>> m_shadowMappings;
};



