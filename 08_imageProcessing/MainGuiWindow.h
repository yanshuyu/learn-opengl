#pragma once
#include<common/GuiWindow.h>
#include<memory>

class ImageProcessingApp;
class LightComponent;
class HemiSphericAmbientComponent;
class Renderer;
class AnimatorController;
class HDRFilterComponent;
class HDRFilterComponent2;
class GaussianBlurFilterComponent;
class SceneObject;


class MainGuiWindow : public GuiWindow {
public:
	MainGuiWindow(const std::string& title, ImageProcessingApp* app);

	bool initialize() override;

	void render() override;

protected: 
	ImageProcessingApp* m_application;
	int m_renderMode = 0;
	std::weak_ptr<LightComponent> m_dirLight;
	std::weak_ptr<LightComponent> m_spotLight;
	std::weak_ptr<LightComponent> m_pointLight;
	std::weak_ptr<HemiSphericAmbientComponent> m_ambientLight;

	std::weak_ptr<AnimatorController> m_animAC;
	float m_speed = 0.f;
	float m_hp = 1.f;

	std::weak_ptr<HDRFilterComponent> m_hdrFilter;
	std::weak_ptr<HDRFilterComponent2> m_hdrFilter2;
	bool m_hdrEnabled;
	float m_exposure;

	std::weak_ptr<GaussianBlurFilterComponent> m_blurFilter;
	bool m_blurEnabled;
	float m_sigma;
	int m_blurKernel;

	SceneObject* m_pbrMan;
};