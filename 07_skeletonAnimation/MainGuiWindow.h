#pragma once
#include<common/GuiWindow.h>

class SkeletonAnimationApp;
class LightComponent;
class Renderer;
class AnimatorController;

class MainGuiWindow : public GuiWindow {
public:
	MainGuiWindow(const std::string& title, SkeletonAnimationApp* app);

	bool initialize() override;

	void render() override;

protected: 
	SkeletonAnimationApp* m_application;
	LightComponent* m_dirLight;
	float m_lightColor[3] = { 0 };
	float m_lightDirection[3] = { 0 };
	float m_lightIntensity = 1.f;
	float m_shadowBias = 0.f;
	int m_renderMode = 0;

	AnimatorController* m_animAC = nullptr;
	float m_speed = 0.f;
};