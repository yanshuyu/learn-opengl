#pragma once
#include"RenderTechnique.h"
#include"RenderTaskExecutor.h"
#include<unordered_map>
#include<memory>

class Buffer;
class FrameBuffer;
class Texture;
class SpotLightShadowMapping;

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

	void clearScrren(int flags) override;

	bool intialize() override;
	void cleanUp() override;

	void prepareForSceneRenderInfo(const SceneRenderInfo_t* si) override;

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
	void pullingRenderTask(ShaderProgram* shader = nullptr) override;

	void onWindowResize(float w, float h) override;
	void onShadowMapResolutionChange(float w, float h);

	RenderPass currentRenderPass() const override {
		return m_currentPass;
	}

	std::string identifier() const override {
		return s_identifier;
	}

private:
	RenderingSettings_t m_renderingSettings;

	ShaderProgram* m_activeShader;
	RenderPass m_currentPass;
	const SceneRenderInfo_t* m_sceneInfo;
	std::unordered_map<RenderPass, std::unique_ptr<RenderTaskExecutor>> m_taskExecutors;

	// light uniform blocks
	std::unique_ptr<Buffer> m_directionalLightUBO;
	std::unique_ptr<Buffer> m_pointLightUBO;
	std::unique_ptr<Buffer> m_spotLightUBO;
	
	// shadow mapping
	std::unique_ptr<SpotLightShadowMapping> m_spotLightShadow;
};