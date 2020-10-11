#pragma once
#include<common/GuiWindow.h>

class ShadowMappingApp;
class LightComponent;
class Renderer;

class LightControlGuiWindow : public GuiWindow {
public:
	LightControlGuiWindow(const std::string& title, ShadowMappingApp* app);

	bool initialize() override;

	void render() override;

protected: 
	ShadowMappingApp* m_application;
	LightComponent* m_dirLight;
	float m_lightColor[3] = { 0 };
	float m_lightDirection[3] = { 0 };
	float m_lightIntensity = 1.f;
	float m_shadowBias = 0.f;
	int m_renderMode = 0;
};