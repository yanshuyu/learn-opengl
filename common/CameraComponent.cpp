#include"CameraComponent.h"
#include"NotificationCenter.h"
#include"SceneObject.h"
#include<glm/gtc/matrix_transform.hpp>
#include<glad/glad.h>

const std::string CameraComponent::s_identifier = "CameraComponent";

CameraComponent::CameraComponent(float wndWidth, float wndHeight) : Component()
, m_windowWidth(wndWidth)
, m_windowHeight(wndHeight)
, m_near(0.1)
, m_far(1000)
, m_fov(60)
, m_backGroundColor(0.f, 0.f, 0.f, 1.f)
, m_viewport()
, m_projMode(CameraComponent::ProjectionMode::Perspective) {
	NotificationCenter::getInstance()->addObserver(this, WindowResizedNotification::s_name, [this](const Notification* n) {
		auto wndResizeNc = static_cast<const WindowResizedNotification*>(n);
		if (wndResizeNc->m_width > 0 && wndResizeNc->m_height > 0)
			this->setWindowSize(wndResizeNc->m_width, wndResizeNc->m_height);
	});
}

CameraComponent::~CameraComponent() {
	NotificationCenter::getInstance()->removeObserver(this);
}

std::string CameraComponent::identifier() const {
	return CameraComponent::s_identifier;
}

Component* CameraComponent::copy() const {
	return nullptr;
}

void CameraComponent::setWindowSize(float w, float h) {
	m_windowWidth = w;
	m_windowHeight = h;
}

glm::mat4 CameraComponent::viewMatrix() const {
	ASSERT(m_owner != nullptr);
	glm::vec3 pos(0.f);
	glm::vec3 right(0.f);
	glm::vec3 forward(0.f, 0.f, -1.f);
	glm::vec3 up(0.f, 1.f, 0.f);
	m_owner->m_transform.getCartesianAxesWorld(&pos, &right, &up, &forward);

	return glm::lookAt(pos, pos + forward, up);
}

glm::mat4 CameraComponent::projectionMatrix() const {
	float x = m_viewport.x * m_windowWidth;
	float y = m_viewport.y * m_windowHeight;
	float w = m_viewport.width * m_windowWidth - x;
	float h = m_viewport.height * m_windowHeight - y;

	if (m_projMode == CameraComponent::ProjectionMode::Perspective)
		return glm::perspective(glm::radians(m_fov), w / h, m_near, m_far);

	return glm::ortho(x, w, y, h);
}

glm::mat4 CameraComponent::viewProjectionMatrix() const {
	return projectionMatrix() * viewMatrix();
}

glm::vec3 CameraComponent::getPosition() const {
#ifdef _DEBUG
	ASSERT(m_owner);
#endif // _DEBUG
	glm::vec3 pos(0);
	m_owner->m_transform.getCartesianAxesWorld(&pos, nullptr, nullptr, nullptr);

	return pos;
}


glm::vec3 CameraComponent::getLookDirection() const {
#ifdef _DEBUG
	ASSERT(m_owner);
#endif // _DEBUG
	glm::vec3 direction(0.f, 0.f, -1.f);
	m_owner->m_transform.getCartesianAxesWorld(nullptr, nullptr, nullptr, &direction);

	return direction;
}


Camera_t CameraComponent::makeCamera() const {
	// unnormalize viewport
	float x = m_viewport.x * m_windowWidth;
	float y = m_viewport.y * m_windowHeight;
	float w = m_viewport.width * m_windowWidth - x;
	float h = m_viewport.height * m_windowHeight - y;

	Camera_t camera;
	camera.viewMatrix = viewMatrix();
	camera.projMatrix = projectionMatrix();
	camera.viewport = Viewport_t(x, y, w, h);
	camera.backgrounColor = m_backGroundColor;
	camera.near = m_near;
	camera.far = m_far;

	return camera;
}