#include"MainGuiWindow.h"
#include<iamgui/imgui.h>
#include"SkeletonAnimationApp.h"
#include"AnimatorController.h"

MainGuiWindow::MainGuiWindow(const std::string& title, SkeletonAnimationApp* app): GuiWindow(title)
, m_application(app)
, m_dirLight() {

}


bool MainGuiWindow::initialize() {
	auto obj = m_application->m_scene->findObjectWithTagRecursive(Scene::Tag::DirectionalLight);
	m_dirLight = obj->getComponent<LightComponent>();
	m_renderMode = int(m_application->m_renderer->getRenderMode()) - 1;
	
	m_animAC = m_application->m_animatorController;

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
	
	if (ImGui::ColorEdit3("Light Color", m_lightColor))
		m_dirLight->setColor({ m_lightColor[0], m_lightColor[1], m_lightColor[2] });
	
	
	if (ImGui::SliderAngle("Light Direction X", m_lightDirection)) {
		auto curRotation = m_dirLight->owner()->m_transform.getRotation();
		m_dirLight->owner()->m_transform.setRotation({ glm::degrees(m_lightDirection[0]),curRotation.y, curRotation.z });
	}


	if (ImGui::SliderAngle("Light Direction Y", m_lightDirection + 1)) {
		auto curRotation = m_dirLight->owner()->m_transform.getRotation();
		m_dirLight->owner()->m_transform.setRotation({ curRotation.x, glm::degrees(m_lightDirection[1]), curRotation.z });
	}


	if (ImGui::SliderAngle("Light Direction Z", m_lightDirection + 2)) {
		auto curRotation = m_dirLight->owner()->m_transform.getRotation();
		m_dirLight->owner()->m_transform.setRotation({ curRotation.x, curRotation.y, glm::degrees(m_lightDirection[2]) });
	}

	if (ImGui::SliderFloat("Light Intensity", &m_lightIntensity, 0.f, 3.f))
		m_dirLight->setIntensity(m_lightIntensity);

	if (ImGui::SliderFloat("Shadow Bias", &m_shadowBias, -0.1f, 0.1f))
		m_dirLight->setShadowBias(m_shadowBias);
	ImGui::Separator();

	ImGui::Text("Animation Setting");

	if (ImGui::SliderFloat("Speed", &m_speed, 0.f, 3.f))
		m_animAC->setSpeed(m_speed);


	ImGui::End();
}