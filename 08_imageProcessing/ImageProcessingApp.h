#pragma once
#include<common/GLApplication.h>
#include"AnimatorController.h"


class ImageProcessingApp : public GLApplication {
	friend class MainGuiWindow;
public:
	ImageProcessingApp(const std::string& wndTitle, int wndWidth = 1920, int wndHeight = 1080, int majorVer = 4, int minorVer = 5);

	void setShadowMapResolution(const glm::vec2& sz) {
		m_renderer->setShadowMapResolution(sz);
	}

	glm::vec2 getShadowMapResolution() const {
		return m_renderer->getShadowMapResolution();
	}

private:
	bool initailize() override;
	void update(double dt) override;
	void render() override;
	void onWindowResized(int width, int height) override;

private:
	void _loadScene();

private:
	std::unique_ptr<Scene> m_scene;
	std::unique_ptr<Renderer> m_renderer;

	std::weak_ptr<AnimatorComponent> m_animator;
};