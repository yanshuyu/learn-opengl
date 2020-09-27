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
, m_left(-20.f)
, m_right(20.f)
, m_top(20.f)
, m_bottom(-20.f)
, m_viewPortX(0.f)
, m_viewPortY(0.f)
, m_viewPortW(1.f)
, m_viewPortH(1.f)
, m_backGroundColor(0.f, 0.f, 0.f, 1.f)
, m_projMode(CameraComponent::ProjectionMode::Perspective) {
	NotificationCenter::getInstance()->addObserver(this, WindowResizedNotification::s_name, [this](const Notification* n) {
		auto wndResizeNc = static_cast<const WindowResizedNotification*>(n);
		if (wndResizeNc->m_width > 0 && wndResizeNc->m_height > 0)
			this->onWindowSizeChange(wndResizeNc->m_width, wndResizeNc->m_height);
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

void CameraComponent::onWindowSizeChange(float w, float h) {
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
	if (m_projMode == CameraComponent::ProjectionMode::Perspective)
		return glm::perspective(glm::radians(m_fov), m_windowWidth / m_windowHeight, m_near, m_far);

	return glm::ortho(m_left, m_right, m_bottom, m_top, m_near, m_far);
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


Viewport_t CameraComponent::getViewPort() const {
	Viewport_t vp;
	vp.x = m_viewPortX * m_windowWidth;
	vp.y = m_viewPortY * m_windowHeight;
	vp.width = m_viewPortW * m_windowWidth - vp.x;
	vp.height = m_viewPortH * m_windowHeight - vp.y;

	return vp;
}


ViewFrustum_t CameraComponent::getViewFrustum() const {
	ViewFrustum_t bb;
	if (m_projMode == ProjectionMode::Orthogonal) {
		bb.ltn = { m_left, m_top, m_near };
		bb.lbn = { m_left, m_bottom, m_near };
		bb.rtn = { m_right, m_top, m_near };
		bb.rbn = { m_right, m_bottom, m_near };

		bb.ltf = { m_left, m_top, m_far };
		bb.lbf = { m_left, m_bottom, m_far };
		bb.rtf = { m_right, m_top, m_far };
		bb.rbf = { m_right, m_bottom, m_far };

	} else if (m_projMode == ProjectionMode::Perspective) {
		float halfHeightNear = m_near * tanf(glm::radians(m_fov));
		float halfWidthNear = m_windowWidth / m_windowHeight * halfHeightNear;
		float halfHeightFar = m_far * tanf(glm::radians(m_fov));
		float halfWidthFar = m_windowWidth / m_windowHeight * halfHeightFar;

		bb.ltn = { -halfWidthNear, halfHeightNear, m_near };
		bb.lbn = { -halfWidthNear, -halfHeightNear, m_near };
		bb.rtn = { halfWidthNear, halfHeightNear, m_near };
		bb.rbn = { halfWidthNear, -halfHeightNear, m_near };

		bb.ltf = { -halfWidthFar, halfHeightFar, m_far };
		bb.lbf = { -halfWidthFar, -halfHeightFar, m_far };
		bb.rtf = { halfWidthFar, halfHeightFar, m_far };
		bb.rbf = { halfWidthFar, -halfHeightFar, m_far };

	} else {
		ASSERT(false);
	}

	bb.applyTransform(m_owner->m_transform.getMatrixWorld());
	
	return bb;
}

