#include"ArcballCameraController.h"
#include<common/SceneObject.h>
#include<common/InputMgr.h>
#include<common/NotificationCenter.h>
#include<glm/glm.hpp>
#include<functional>


COMPONENT_IDENTIFIER_IMP(ArcballCameraController, "ArcballCameraController");

ArcballCameraController::ArcballCameraController() {
	NotificationCenter::getInstance()->addObserver(this, 
		MouseScrollNotification::s_name, 
		std::bind(&ArcballCameraController::onMouseScrolling, this, std::placeholders::_1));
}

ArcballCameraController::~ArcballCameraController() {
	NotificationCenter::getInstance()->removeObserver(this);
}

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

	if (input->isMouseButtonPressed(MouseButtonCode::Left) /*&& input->isKeyPressed(KeyCode::LCtrl)*/) {
		auto cursorPos = input->getCursorPosition();
		m_theta += (cursorPos.first - m_lastMouseXPos) * m_moveSpeed * float(dt);
		m_phi += (cursorPos.second - m_lastMouseYPos) * m_moveSpeed * float(dt);
		m_phi = clamp(m_phi, 0.f, 80.f);

		glm::vec3 pos = sphericalToCartesian(m_radius, glm::radians(m_theta), glm::radians(m_phi));
		m_owner->m_transform.setPosition(m_target + pos);
		m_owner->m_transform.lookAt(m_target - pos, { 0.f, 1.f, 0.f });

		m_lastMouseXPos = cursorPos.first;
		m_lastMouseYPos = cursorPos.second;
	}

	if (input->isMouseButtonUp(MouseButtonCode::Left)) {
		m_lastMouseXPos = 0.f;
		m_lastMouseYPos = 0.f;
	}
}


void ArcballCameraController::setPosition(const glm::vec3& pos) {
	auto coord = cartesianToSpherical(pos);
	m_radius = coord.x;
	m_theta = coord.y;
	m_phi = coord.z;
}


void ArcballCameraController::onMouseScrolling(const Notification* nc) {
	auto mouseScrollNC = static_cast<const MouseScrollNotification*>(nc);
	m_radius += mouseScrollNC->m_yOffset;
	glm::vec3 pos = sphericalToCartesian(m_radius, glm::radians(m_theta), glm::radians(m_phi));
	m_owner->m_transform.setPosition(m_target + pos);
}

std::string ArcballCameraController::identifier() const {
	return s_identifier;
}

Component* ArcballCameraController::copy() const {
	return nullptr;
}


glm::vec3 ArcballCameraController::sphericalToCartesian(float radius, float theta, float phi) {
	float x = radius * cosf(phi) * cosf(theta);
	float y = radius * sinf(phi);
	float z = radius * cosf(phi) * sinf(theta);

	return glm::vec3(x, y, z);
}


glm::vec3 ArcballCameraController::cartesianToSpherical(const glm::vec3& pos) {
	float radius, theta, phi;
	glm::vec3 t2v = pos - m_target;
	radius = glm::length(t2v);
	theta = glm::degrees(atanf(t2v.z / t2v.x));
	phi = glm::degrees(asinf(t2v.y / radius));

	return glm::vec3(radius, theta, phi);
}