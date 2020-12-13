#include"MainGuiWindow.h"
#include"ImageProcessingApp.h"
#include<iamgui/imgui.h>
#include"AnimatorController.h"
#include<common/HDRFilter2.h>
#include<common/GaussianBlurFilter.h>

MainGuiWindow::MainGuiWindow(const std::string& title, ImageProcessingApp* app): GuiWindow(title)
, m_application(app)
, m_dirLight()
, m_spotLight()
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

	obj = m_application->m_scene->findObjectWithTagRecursive(Scene::Tag::SpotLight);
	if (obj) {
		m_spotLight = obj->getComponent<LightComponent>();
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
	ImGui::Text("Lightting Setting");
	
	static float envSkyColor[3] = { m_application->m_scene->getEnviromentLight().first.r, m_application->m_scene->getEnviromentLight().first.g, m_application->m_scene->getEnviromentLight().first.b };
	static float envGroundColor[3] = { m_application->m_scene->getEnviromentLight().second.r, m_application->m_scene->getEnviromentLight().second.g, m_application->m_scene->getEnviromentLight().second.b };
	
	if (ImGui::ColorEdit3("Ambient Sky", envSkyColor))
		m_application->m_scene->setEnviromentLight({ envSkyColor[0], envSkyColor[1], envSkyColor[2] }, { envGroundColor[0], envGroundColor[1], envGroundColor[2] });

	if (ImGui::ColorEdit3("Amibient Ground", envGroundColor))
		m_application->m_scene->setEnviromentLight({ envSkyColor[0], envSkyColor[1], envSkyColor[2] }, { envGroundColor[0], envGroundColor[1], envGroundColor[2] });

	if (!m_dirLight.expired() || !m_spotLight.expired()) {
		if (!m_dirLight.expired()) {
			auto light = m_dirLight.lock();			
			static bool enabled = light->m_isEnable;
			static float lightColor[3] = { light->getColor().r, light->getColor().g, light->getColor().b };
			static float rotation[3] = { light->owner()->m_transform.getRotation().x, light->owner()->m_transform.getRotation().y, light->owner()->m_transform.getRotation().z };
			static float intensity = light->getIntensity();
			static const char* shadowTypeLables[3] = { "No Shadow", "Hard Shadow", "Soft Shadow" };
			static int shadowType = int(light->getShadowType());
			static float shadowStren = light->getShadowStrength();
			static float shadowBias = light->getShadowBias();
			
			ImGui::PushID("Direction Light");

			if (ImGui::Checkbox("Directional Light", &enabled)) {
				light->m_isEnable = enabled;
			}

			if (ImGui::DragFloat3("Rotation", rotation, 1, -360, 360)) {
				light->owner()->m_transform.setRotation({ rotation[0], rotation[1], rotation[2] });
			}

			if (ImGui::ColorEdit3("Color", lightColor))
				light->setColor({ lightColor[0], lightColor[1], lightColor[2] });

			if (ImGui::SliderFloat("Intensity", &intensity, 0.f, 10.f))
				light->setIntensity(intensity);

			if (ImGui::Combo("Shadow Type", &shadowType, shadowTypeLables, IM_ARRAYSIZE(shadowTypeLables)))
				light->setShadowType(ShadowType(shadowType));

			if (ImGui::SliderFloat("Shadow Strength", &shadowStren, 0.1f, 1.f))
				light->setShadowStrength(shadowStren);

			if (ImGui::SliderFloat("Shadow Bias", &shadowBias, -0.1f, 0.1f))
				light->setShadowBias(shadowBias);

			ImGui::PopID();
		}


		if (!m_spotLight.expired()) {
			auto light = m_spotLight.lock();
			static bool enabled = light->m_isEnable;

			static float lightColor[3] = { light->getColor().r, light->getColor().g, light->getColor().b };
			static float pos[3] = { light->owner()->m_transform.getPosition().x, light->owner()->m_transform.getPosition().y, light->owner()->m_transform.getPosition().z };
			static float rotation[3] = { light->owner()->m_transform.getRotation().x, light->owner()->m_transform.getRotation().y, light->owner()->m_transform.getRotation().z };
			static float angles[2] = { light->getSpotInnerAngle(), light->getSpotOutterAngle() };
			static float intensity = light->getIntensity();
			static float range = light->getRange();
			static const char* shadowTypeLables[3] = { "No Shadow", "Hard Shadow", "Soft Shadow" };
			static int shadowType = int(light->getShadowType());
			static float shadowBias = light->getShadowBias();

			ImGui::PushID("Spot Light");

			if (ImGui::Checkbox("Spot Light", &enabled))
				light->m_isEnable = enabled;

			if (ImGui::ColorEdit3("Color", lightColor))
				light->setColor({ lightColor[0], lightColor[1], lightColor[2] });

			if (ImGui::InputFloat3("Position", pos, 2))
				light->getGameObject()->m_transform.setPosition({ pos[0], pos[1], pos[2] });

			if (ImGui::DragFloat3("Rotation", rotation, 1, -360, 360))
				light->owner()->m_transform.setRotation({ rotation[0], rotation[1], rotation[2] });

			if (ImGui::DragFloat2("Angles", angles, 1, 0, 180)) {
				light->setSpotInnerAngle(angles[0]);
				light->setSpotOutterAngle(angles[1]);
			}

			if (ImGui::SliderFloat("Range", &range, 0, 300))
				light->setRange(range);

			if (ImGui::SliderFloat("Intensity", &intensity, 0.f, 10.f))
				light->setIntensity(intensity);

			if (ImGui::Combo("Shadow Type", &shadowType, shadowTypeLables, IM_ARRAYSIZE(shadowTypeLables)))
				light->setShadowType(ShadowType(shadowType));

			if (ImGui::SliderFloat("Shadow Bias", &shadowBias, -0.1f, 0.1f))
				light->setShadowBias(shadowBias);

			ImGui::PopID();
		}

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