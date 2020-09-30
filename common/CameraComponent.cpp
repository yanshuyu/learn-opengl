#include"CameraComponent.h"
#include"SceneObject.h"
#include<glm/gtc/matrix_transform.hpp>
#include<glad/glad.h>

const std::string CameraComponent::s_identifier = "CameraComponent";

CameraComponent::CameraComponent(float ar) : Component()
, m_aspectRatio(ar)
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
}

CameraComponent::~CameraComponent() {
}

std::string CameraComponent::identifier() const {
	return CameraComponent::s_identifier;
}

Component* CameraComponent::copy() const {
	return nullptr;
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
		return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_near, m_far);

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


Viewport_t CameraComponent::getViewPort(float renderWidth, float renderHeight) const {
	Viewport_t vp;
	vp.x = m_viewPortX * renderWidth;
	vp.y = m_viewPortY * renderHeight;
	vp.width = m_viewPortW * renderWidth - vp.x;
	vp.height = m_viewPortH * renderHeight - vp.y;

	return vp;
}


ViewFrustum_t CameraComponent::getViewFrustum() const {
	ViewFrustum_t vf;
	if (m_projMode == ProjectionMode::Orthogonal) {
		vf.points[ViewFrustum_t::PointIndex::LTN] = { m_left, m_top, m_near };
		vf.points[ViewFrustum_t::PointIndex::LBN] = { m_left, m_bottom, m_near };
		vf.points[ViewFrustum_t::PointIndex::RTN] = { m_right, m_top, m_near };
		vf.points[ViewFrustum_t::PointIndex::RBN] = { m_right, m_bottom, m_near };

		vf.points[ViewFrustum_t::PointIndex::LTF] = { m_left, m_top, m_far };
		vf.points[ViewFrustum_t::PointIndex::LBF] = { m_left, m_bottom, m_far };
		vf.points[ViewFrustum_t::PointIndex::RTF] = { m_right, m_top, m_far };
		vf.points[ViewFrustum_t::PointIndex::RBF] = { m_right, m_bottom, m_far };

	} else if (m_projMode == ProjectionMode::Perspective) {
		float xn = m_near * tanf(glm::radians(m_fov * 0.5f));
		float yn = xn / m_aspectRatio;
		
		float xf = m_far * tanf(glm::radians(m_fov * 0.5f));
		float yf = xf / m_aspectRatio;
	

		vf.points[ViewFrustum_t::PointIndex::LTN] = { -xn, yn, -m_near };
		vf.points[ViewFrustum_t::PointIndex::LBN] = { -xn, -yn, -m_near };
		vf.points[ViewFrustum_t::PointIndex::RTN] = { xn, yn, -m_near };
		vf.points[ViewFrustum_t::PointIndex::RBN] = { xn, --yn, -m_near };

		vf.points[ViewFrustum_t::PointIndex::LTF] = { -xf, yf, -m_far };
		vf.points[ViewFrustum_t::PointIndex::LBF] = { -xf, -yf, -m_far };
		vf.points[ViewFrustum_t::PointIndex::RTF] = { xf, yf, -m_far };
		vf.points[ViewFrustum_t::PointIndex::RBF] = { xf, -yf, -m_far };

	} else {
		ASSERT(false);
	}
	
	return vf;
}


Camera_t CameraComponent::makeCamera(float wndWidth, float wndHeight) const {
	Camera_t c;
	c.position = getPosition();
	c.lookDirection = getLookDirection();
	c.fov = glm::radians(m_fov);
	c.aspectRatio = m_aspectRatio;
	c.near = m_near;
	c.far = m_far;
	c.backgrounColor = m_backGroundColor;
	c.viewport = getViewPort(wndWidth, wndHeight);
	c.viewFrustum = getViewFrustum();
	c.viewMatrix = viewMatrix();
	c.projMatrix = projectionMatrix();
	
	return c;
}