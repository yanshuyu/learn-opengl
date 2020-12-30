#pragma once
#include"RenderTechnique.h"
#include"RenderTaskExecutor.h"
#include"RenderTarget.h"


class Buffer;
class Texture;
class IShadowMapping;

#define MAX_NUM_LIGHTS 1024

class ForwardPlusRenderer : public RenderTechniqueBase {
	struct Light {
		glm::vec4 position; // (xyz)position (w)range
		glm::vec4 color; // (rgb)color (a)intensity
		glm::vec3 direction;
		float _padding;
		glm::vec3 angles; // spot angles/ ambient ground color
		int type;

		Light() : position(0.f)
			, color(0.f)
			, direction(0.f)
			, angles(0.f)
			, type(0) {

		}
	};

public:
	ForwardPlusRenderer(Renderer* renderer);
	~ForwardPlusRenderer();

	ForwardPlusRenderer(const ForwardPlusRenderer& other) = delete;
	ForwardPlusRenderer(ForwardPlusRenderer&& rv) = delete;
	ForwardPlusRenderer& operator = (const ForwardPlusRenderer& other) = delete;
	ForwardPlusRenderer& operator = (ForwardPlusRenderer&& rv) = delete;

	static const std::string s_identifier;
	
	inline std::string identifier() const override {
		return s_identifier;
	}

	inline Texture* getRenderedFrame() override {
		return m_outputTarget.getAttachedTexture(RenderTarget::Slot::Color);
	}

	bool intialize() override;
	void cleanUp() override;

	void beginFrame() override;
	void endFrame() override;

	void render(const MeshRenderItem_t& task) override;

	void drawDepthPass(const Scene_t& scene) override;
	void drawGeometryPass(const Scene_t& scene) override {};
	void drawOpaquePass(const Scene_t& scene) override;
	void drawTransparentPass(const Scene_t& scene) override {};

	void onWindowResize(float w, float h) override;
	void onShadowMapResolutionChange(float w, float h) override;

protected:
	inline void DrawOpaques(const Scene_t& scene, bool useCutout);
	void RenderMainLights(const Scene_t& scene);
	void PrepareLights(const Scene_t& scene);
	void genShadowMap(const Scene_t& scene, const Light_t& light);

protected:
	GPUPipelineState m_depthPassPipelineState;
	GPUPipelineState m_shadowPassPipelineState;
	GPUPipelineState m_lightPassPipelineState;
	GPUPipelineState m_opaqusPipelineState;
	GPUPipelineState m_cutoutPipelineState;

	// default frame rener target (input frame for post processing)
	RenderTarget m_outputTarget;

	std::unordered_map<RenderPass, std::unique_ptr<RenderTaskExecutor>> m_taskExecutors;

	// shadow mapping
	std::unordered_map<LightType, std::unique_ptr<IShadowMapping>> m_shadowMappings;

	// lights SSBO
	std::unique_ptr<Buffer> m_lightsSSBO;
	std::array<Light, MAX_NUM_LIGHTS> m_lights;
};