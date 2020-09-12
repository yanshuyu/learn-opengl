#include"FirstPersonCameraController.h"
#include<common/SceneObject.h>
#include<common/InputMgr.h>
#include<glm/glm.hpp>



const std::string FirstPersonCameraController::s_identifier = "FirstPersonCameraController";


FirstPersonCameraController* FirstPersonCameraController::create() {
	return new FirstPersonCameraController();
}

void FirstPersonCameraController::destory(FirstPersonCameraController* c) {
	if (c) {
		delete c;
		c = nullptr;
	}
}

void FirstPersonCameraController::update(double dt) {
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
		yRot = (m_lastMouseXPos - cursorPos.first) * m_moveSpeed * dt;
		xRot = (m_lastMouseYPos - cursorPos.second) * m_moveSpeed * dt;

		m_lastMouseXPos = cursorPos.first;
		m_lastMouseYPos = cursorPos.second;
	}

	if (input->isMouseButtonUp(MouseButtonCode::Left)) {
		m_lastMouseXPos = 0.f;
		m_lastMouseYPos = 0.f;
	}

	if (fabs(xRot) > 0.1 || fabs(yRot) > 0.1) {
		m_owner->m_transform.rotateBy(glm::vec3(xRot, yRot, 0.f));
		m_owner->m_transform.applyTransform();
	}

	glm::vec3 direction(0.f);

	if (input->isKeyPressed(KeyCode::A)) {
		direction.x = -1.f;
	}

	if (input->isKeyPressed(KeyCode::D)) {
		direction.x = 1.f;
	}

	if (input->isKeyPressed(KeyCode::W)) {
		direction.z = -1.f;
	}

	if (input->isKeyPressed(KeyCode::S)) {
		direction.z = 1.f;
	}

	if (input->isKeyPressed(KeyCode::Q)) {
		direction.y = -1;
	}

	if (input->isKeyPressed(KeyCode::E)) {
		direction.y = 1;
	}


	float l = glm::length(direction);
	if (l > 0) {
		m_owner->m_transform.translateBy(glm::normalize(direction) * m_moveSpeed * float(dt));
		m_owner->m_transform.applyTransform();
	}

	
	if (input->isKeyPressed(KeyCode::ESC)) {
		m_owner->m_transform.resetTransform();
	}
}

std::string FirstPersonCameraController::indentifier() const {
	return s_identifier;
}

Component* FirstPersonCameraController::copy() const {
	return nullptr;
}

