#pragma once
#include<common/GLApplication.h>


class ModelLoadingApp : public GLApplication {
public:
	ModelLoadingApp(const std::string& wndTitle, int wndWidth = 1920, int wndHeight = 1080);

private:
	bool initailize() override;
	void update(double dt) override;
	void render() override;

private:
	std::unique_ptr<Scene> m_scene;
	std::unique_ptr<Renderer> m_renderer;
};