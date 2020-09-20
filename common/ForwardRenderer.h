#pragma once
#include"RenderTechnique.h"
#include"RenderTaskExecutor.h"
#include<unordered_map>
#include<memory>

class Buffer;

class ForwardRenderer : public RenderTechnique {
	struct DirectionalLightBlock {
		glm::vec4 color;
		glm::vec3 inverseDiretion;
	};

public:
	ForwardRenderer();

	ForwardRenderer(const ForwardRenderer& other) = delete;
	ForwardRenderer(ForwardRenderer&& rv) = delete;
	ForwardRenderer& operator = (const ForwardRenderer& other) = delete;
	ForwardRenderer& operator = (ForwardRenderer&& rv) = delete;

	void clearScrren(int flags) override;

	bool intialize() override;
	void cleanUp() override;

	void prepareForSceneRenderInfo(const SceneRenderInfo_t& si) override;

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

	RenderPass currentRenderPass() const override;

	void performTask(const RenderTask_t& task) override;

private:
	void beginDirectionalLightPass(const Light_t& l);
	void endDirectionalLightPass();

private:
	ShaderProgram* m_activeShader;
	RenderPass m_currentPass;
	SceneRenderInfo_t m_sceneInfo;

	std::unique_ptr<Buffer> m_directionalLightUBO;
	
	std::unordered_map<RenderPass, std::unique_ptr<RenderTaskExecutor>> m_taskExecutors;
};