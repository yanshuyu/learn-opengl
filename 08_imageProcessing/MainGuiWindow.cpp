#include"MainGuiWindow.h"
#include"ImageProcessingApp.h"
#include<iamgui/imgui.h>
#include"AnimatorController.h"
#include<common/HDRFilter2.h>
#include<common/GaussianBlurFilter.h>

MainGuiWindow::MainGuiWindow(const std::string& title, ImageProcessingApp* app): GuiWindow(title)
, m_application(app)
, m_dirLight()
, m_animAC()
, m_hdrFilter()
, m_blurFilter() {

}


bool MainGuiWindow::initialize() {
	auto obj = m_application->m_scene->findObjectWithTagRecursive(Scene::Tag::DirectionalLight);
	
	if (obj) {
		m_dirLight = obj->getComponent<LightComponent>();
		m_renderMode = int(m_application->m_renderer->getRenderMode()) - 1;
	}
	
	obj = m_application->m_scene->findObjectWithTagRecursive(100);
	if (obj) {
		m_animAC = obj->getComponent<AnimatorController>();
	}

	obj = m_application->m_scene->findObjectWithTagRecursive(Scene::Tag::Camera);
	if (obj) {
		m_hdrFilter = obj->getComponent<HDRFilterComponent2>();
		if (!m_hdrFilter.expired()) {
			m_hdrEnabled = m_hdrFilter.lock()->m_isEnable;
			m_exposure = m_hdrFilter.lock()->getExposure();
		}

		m_blurFilter = obj->getComponent<GaussianBlurFilterComponent>();
		if (!m_blurFilter.expired()) {
			auto filter = m_blurFilter.lock();
			m_sigma = filter->getSigma();
			m_blurKernel = filter->getKernelSize();
			m_blurEnabled = filter->m_isEnable;
		}
	}

	return true;
}

void MainGuiWindow::render() {
	ImGui::Begin(m_title.c_str());

	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::Separator();

	ImGui::Text("Rendering Setting");
	
	static const char* modes[] = { "Forward", "Deffered" };
	if (ImGui::Combo("Render Mode", &m_renderMode, modes, IM_ARRAYSIZE(modes)))
		m_application->m_renderer->setRenderMode(Renderer::Mode(m_renderMode + 1));
	ImGui::Separator();

	if (!m_dirLight.expired()) {
		ImGui::Text("Lightting Setting");

		if (ImGui::ColorEdit3("Light Color", m_lightColor) && !m_dirLight.expired())
			m_dirLight.lock()->setColor({ m_lightColor[0], m_lightColor[1], m_lightColor[2] });


		if (ImGui::SliderAngle("Light Direction X", m_lightDirection) && !m_dirLight.expired()) {
			auto curRotation = m_dirLight.lock()->owner()->m_transform.getRotation();
			m_dirLight.lock()->owner()->m_transform.setRotation({ glm::degrees(m_lightDirection[0]),curRotation.y, curRotation.z });
		}


		if (ImGui::SliderAngle("Light Direction Y", m_lightDirection + 1) && !m_dirLight.expired()) {
			auto curRotation = m_dirLight.lock()->owner()->m_transform.getRotation();
			m_dirLight.lock()->owner()->m_transform.setRotation({ curRotation.x, glm::degrees(m_lightDirection[1]), curRotation.z });
		}


		if (ImGui::SliderAngle("Light Direction Z", m_lightDirection + 2) && !m_dirLight.expired()) {
			auto curRotation = m_dirLight.lock()->owner()->m_transform.getRotation();
			m_dirLight.lock()->owner()->m_transform.setRotation({ curRotation.x, curRotation.y, glm::degrees(m_lightDirection[2]) });
		}

		if (ImGui::SliderFloat("Light Intensity", &m_lightIntensity, 0.f, 3.f) && !m_dirLight.expired())
			m_dirLight.lock()->setIntensity(m_lightIntensity);

		if (ImGui::SliderFloat("Shadow Bias", &m_shadowBias, -0.1f, 0.1f) && !m_dirLight.expired())
			m_dirLight.lock()->setShadowBias(m_shadowBias);
		ImGui::Separator();
	}

	if (!m_animAC.expired()) {
		ImGui::Text("Animation Setting");

		if (ImGui::SliderFloat("Speed", &m_speed, 0.f, 3.f) && !m_animAC.expired())
			m_animAC.lock()->setSpeed(m_speed);

		if (ImGui::SliderFloat("HP", &m_hp, 0.f, 1.f) && !m_animAC.expired())
			m_animAC.lock()->setHp(m_hp);

	}


	if (!m_hdrFilter.expired()) {
		auto hdrFilter = m_hdrFilter.lock();
		if (ImGui::Checkbox("HDR Filter", &m_hdrEnabled))
			hdrFilter->m_isEnable = m_hdrEnabled;

		if (ImGui::SliderFloat("Exposure", &m_exposure, 0.f, 3.f))
			hdrFilter->setExposure(m_exposure);
	}


	if (!m_blurFilter.expired()) {
		auto blurFilter = m_blurFilter.lock();
		if (ImGui::Checkbox("Gaussian Blur Filter", &m_blurEnabled))
			blurFilter->m_isEnable = m_blurEnabled;

		if (ImGui::SliderInt("Kernel", &m_blurKernel, blurFilter->sMinKernel, blurFilter->sMaxKernel)) {
			int oddKernel = m_blurKernel % 2 == 0 ? m_blurKernel - 1 : m_blurKernel;
			blurFilter->setKernelSize(oddKernel);
		}

		if (ImGui::SliderFloat("Sigma", &m_sigma, blurFilter->sMinSigma, blurFilter->sMaxSigma))
			blurFilter->setsigma(m_sigma);
	}

	ImGui::End();
}