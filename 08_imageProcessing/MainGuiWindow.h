#pragma once
#include<common/GuiWindow.h>
#include<memory>

class ImageProcessingApp;
class LightComponent;
class Renderer;
class AnimatorController;

class MainGuiWindow : public GuiWindow {
public:
	MainGuiWindow(const std::string& title, ImageProcessingApp* app);

	bool initialize() override;

	void render() override;

protected: 
	ImageProcessingApp* m_application;
	std::weak_ptr<LightComponent> m_dirLight;
	float m_lightColor[3] = { 0 };
	float m_lightDirection[3] = { 0 };
	float m_lightIntensity = 1.f;
	float m_shadowBias = 0.f;
	int m_renderMode = 0;

	std::weak_ptr<AnimatorController> m_animAC;
	float m_speed = 0.f;
	float m_hp = 1.f;
};