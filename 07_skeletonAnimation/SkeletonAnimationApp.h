#pragma once
#include<common/GLApplication.h>
#include"AnimatorController.h"


class SkeletonAnimationApp : public GLApplication {
	friend class MainGuiWindow;
public:
	SkeletonAnimationApp(const std::string& wndTitle, int wndWidth = 1920, int wndHeight = 1080);
	
private:
	bool initailize() override;
	void update(double dt) override;
	void render() override;      
	void debugDraw(double dt) override;
	void onWindowResized(int width, int height) override;

private:
	std::unique_ptr<Scene> m_scene;
	std::unique_ptr<Renderer> m_renderer;

	std::unique_ptr<ScalarTrack> m_scalarTrack;
	std::unique_ptr<ScalarTrack> m_scalarTrackCubic;
	
 	std::weak_ptr<AnimatorComponent> m_animator;
};