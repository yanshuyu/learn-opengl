#pragma once
#include<common/GLApplication.h>


class ForwardRendererApp : public GLApplication {
public:
	ForwardRendererApp(const std::string& wndTitle, int wndWidth = 1920, int wndHeight = 1080);

private:
	bool initailize() override;
	void update(double dt) override;
	void render() override;
	void onWindowResized(int width, int height) override;

private:
	std::unique_ptr<Scene> m_scene;
	std::unique_ptr<Renderer> m_renderer;
};