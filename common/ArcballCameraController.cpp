#include"ArcballCameraController.h"
#include<common/SceneObject.h>
#include<common/InputMgr.h>
#include<glm/glm.hpp>



const std::string ArcballCameraController::s_identifier = "ArcballCameraController";


ArcballCameraController* ArcballCameraController::create() {
	return new ArcballCameraController();
}

void ArcballCameraController::destory(ArcballCameraController* c) {
	if (c) {
		delete c;
		c = nullptr;
	}
}

void ArcballCameraController::update(double dt) {
	auto input = InputManager::getInstance();

	float xRot = 0.f;
	float yRot = 0.f;

	if (input->isMouseButtonDown(MouseButtonCode::Left)) {
		auto cursorPos = input->getCursorPosition();
		m_lastMouseXPos = cursorPos.first;
		m_lastMouseYPos = cursorPos.second;
	}

	if (input->isMouseButtonPressed(MouseButtonCode::Left)) {
		auto cursorPos = input->getCursorPosition();
		m_phi += (m_lastMouseXPos - cursorPos.first) * m_moveSpeed * dt;
		m_theta += (m_lastMouseYPos - cursorPos.second) * m_moveSpeed * dt;

		m_owner->m_transform.setPosition(sphericalToCartesian(glm::vec3(m_radius, m_phi, m_theta)));
		//m_owner->m_transform.lookAt(glm::vec3(0, 0, 0));

		m_lastMouseXPos = cursorPos.first;
		m_lastMouseYPos = cursorPos.second;
	}

	if (input->isMouseButtonUp(MouseButtonCode::Left)) {
		m_lastMouseXPos = 0.f;
		m_lastMouseYPos = 0.f;
	}


	if (input->isKeyPressed(KeyCode::ESC)) {
		m_owner->m_transform.resetTransform();
	}
}

std::string ArcballCameraController::identifier() const {
	return s_identifier;
}

Component* ArcballCameraController::copy() const {
	return nullptr;
}


glm::vec3 ArcballCameraController::cartesianToSpherical(const glm::vec3& p) {
	glm::vec3 pow2  = p * p;
	float r = glm::sqrt(pow2.x + pow2.y + pow2.z);
	float phi = glm::atan(p.z / p.x);
	float theta = glm::acos(p.y / 2);

	return glm::vec3(r, phi, theta);
}



glm::vec3 ArcballCameraController::sphericalToCartesian(const glm::vec3& p) {
	float x = p.x * glm::cos(p.z) * glm::cos(p.y);
	float y = p.x * glm::sin(p.z);
	float z = p.x * glm::cos(p.z) * glm::sin(p.y);

	return glm::vec3(x, y, z);
}