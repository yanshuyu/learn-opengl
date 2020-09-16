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
	//m_owner->m_transform.setTransformMode(TransformComponent::TransformMode::World);
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
	}

	glm::vec3 direction(0.f);
	TransformComponent& transform = m_owner->m_transform;

	if (input->isKeyPressed(KeyCode::A)) {
		glm::vec3 detal = -transform.getRight() * m_moveSpeed * float(dt);
		transform.translateBy(detal);
	}

	if (input->isKeyPressed(KeyCode::D)) {
		glm::vec3 detal = transform.getRight() * m_moveSpeed * float(dt);
		transform.translateBy(detal);
	}

	if (input->isKeyPressed(KeyCode::W)) {
		glm::vec3 detal = transform.getForward() * m_moveSpeed * float(dt);
		transform.translateBy(detal);
	}

	if (input->isKeyPressed(KeyCode::S)) {
		glm::vec3 detal = -transform.getForward() * m_moveSpeed * float(dt);
		transform.translateBy(detal);
	}

	if (input->isKeyPressed(KeyCode::Q)) {
		glm::vec3 detal = transform.getUp() * m_moveSpeed * float(dt);
		transform.translateBy(detal);
	}

	if (input->isKeyPressed(KeyCode::E)) {
		glm::vec3 detal = -transform.getUp() * m_moveSpeed * float(dt);
		transform.translateBy(detal);
	}

	
	if (input->isKeyPressed(KeyCode::ESC)) {
		m_owner->m_transform.resetTransform();
	}
}

std::string FirstPersonCameraController::identifier() const {
	return s_identifier;
}

Component* FirstPersonCameraController::copy() const {
	return nullptr;
}

