#pragma once
#include"Renderer.h"



class ForwardRenderer : public Renderer {
public:
	ForwardRenderer();

	bool initialize() override;
	void renderScene(Scene* s) override;
	void subsimtTask(const RenderTask_t& task) override;

protected:
	void beginUnlitPass() override;
	void endUnlitPass() override;

	void beginLightpass(const Light_t& light) override;
	void endLightPass() override;

private:
	void prepareTask(const RenderTask_t& task);
	void performTask(const RenderTask_t& task);

private:
	RenderContext m_renderContext;
	Camera_t m_camera;
	Scene* m_renderingScene;
	std::shared_ptr<ShaderProgram> m_unlitShader;

};