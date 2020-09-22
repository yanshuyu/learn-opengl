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

	struct PointLightBlock {
		glm::vec4 position;
		glm::vec4 color;
	};

	struct SpotLightBlock {
		glm::vec4 position;
		glm::vec4 color;
		glm::vec3 inverseDirection;
		glm::vec2 angles;
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

	RenderPass currentRenderPass() const override;

	void performTask(const RenderTask_t& task) override;

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