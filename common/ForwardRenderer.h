#pragma once
#include"RenderTechnique.h"
#include"RenderTaskExecutor.h"
#include<unordered_map>
#include<memory>

class Buffer;

class ForwardRenderer : public RenderTechnique {
	friend class ZPassRenderTaskExecutor;
	friend class UlitPassRenderTaskExecutror;
	friend class LightPassRenderTaskExecuter;

public:
	ForwardRenderer();
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
	bool shouldVisitScene() const override;

	void beginFrame() override;
	void endFrame() override;

	void beginDepthPass() override;
	void endDepthPass() override;

	void beginGeometryPass() override;
	void endGeometryPass() override;

	void beginUnlitPass() override;
	void endUnlitPass() override;

	void beginLightPass(const Light_t& l) override;
	void endLightPass(const Light_t& l) override;

	void beginTransparencyPass() override;
	void endTransparencyPass() override;

	void performTask(const RenderTask_t& task) override;

	void onWindowResize(float w, float h) override {

	}

	RenderPass currentRenderPass() const override {
		return m_currentPass;
	}

	std::string identifier() const override {
		return s_identifier;
	}

private:
	void beginDirectionalLightPass(const Light_t& l);
	void endDirectionalLightPass();

	void beginPointLightPass(const Light_t& l);
	void endPointLightPass();

	void beginSpotLightPass(const Light_t& l);
	void endSpotLightPass();

private:
	ShaderProgram* m_activeShader;
	RenderPass m_currentPass;
	const SceneRenderInfo_t* m_sceneInfo;

	std::unique_ptr<Buffer> m_directionalLightUBO;
	std::unique_ptr<Buffer> m_pointLightUBO;
	std::unique_ptr<Buffer> m_spotLightUBO;
	
	std::unordered_map<RenderPass, std::unique_ptr<RenderTaskExecutor>> m_taskExecutors;
};