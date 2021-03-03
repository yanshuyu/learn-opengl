#pragma once
#include"RenderTechnique.h"
#include"RenderTaskExecutor.h"
#include"RenderTarget.h"
#include"Renderer.h"

class Buffer;
class Texture;
class IShadowMapping;

class ForwardPlusRenderer : public RenderTechniqueBase {

	const unsigned MAX_FRAG_PER_PIXEL = 32;

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
			, type(0) {	}
	};

	struct Fragment {
		glm::vec4 color;
		float depth;
		unsigned int next;
		float _padding[2];

		Fragment() : color(0.f)
			, depth(1.f)
			, next(0xffffffff) { }
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

	inline Texture* getRenderedFrame() override;

	bool intialize() override;
	void cleanUp() override;

	void beginFrame() override;
	void endFrame() override;

	void render(const MeshRenderItem_t& task) override;

	void drawDepthPass(const Scene_t& scene) override;
	void drawGeometryPass(const Scene_t& scene) override {};
	void drawOpaquePass(const Scene_t& scene) override;
	void drawTransparentPass(const Scene_t& scene) override;

	void onWindowResize(float w, float h) override;
	void onShadowMapResolutionChange(float w, float h) override;

protected:
	void setupPipelineStates();
	bool setupOutputTarget();

	void DrawOpaques(const Scene_t& scene, bool useCutout);
	void RenderMainLights(const Scene_t& scene);
	void PrepareLights(const Scene_t& scene);
	void genShadowMap(const Scene_t& scene, const Light_t& light);


	bool setupOIT();
	void cleanOIT();
	void genOITFragList(const Scene_t& scene);
	void blendOITFragList();

protected:
	GPUPipelineState m_depthPassPipelineState;
	GPUPipelineState m_shadowPassPipelineState;
	GPUPipelineState m_lightPassPipelineState;
	GPUPipelineState m_opaqusPipelineState;
	GPUPipelineState m_cutoutPipelineState;
	GPUPipelineState m_oitDrawPipelineState;
	GPUPipelineState m_oitBlendPipelineState;

	// default frame rener target (input frame for post processing)
	std::unique_ptr<RenderTarget> m_outputTarget;

	std::unordered_map<RenderPass, std::unique_ptr<RenderTaskExecutor>> m_taskExecutors;

	// shadow mapping
	std::unordered_map<LightType, std::unique_ptr<IShadowMapping>> m_shadowMappings;

	// lights SSBO
	std::unique_ptr<Buffer> m_lightsSSBO;
	std::array<Light, MAX_NUM_TOTAL_LIGHTS> m_lights;

	// order indepamdent transparency
	std::unique_ptr<Buffer> m_fragIdxACBO;
	std::unique_ptr<Buffer> m_fragListSSBO;
	std::unique_ptr<Buffer> m_fragListHeaderResetBuffer;
	std::unique_ptr<Texture> m_fragListHeader;
	std::unique_ptr<RenderTarget> m_oitBlendTarget;
	bool m_isOITSetup;
};