#include"CameraComponent.h"
#include"SceneObject.h"
#include"Scene.h"
#include"Renderer.h"
#include<glm/gtc/matrix_transform.hpp>
#include<glad/glad.h>

RTTI_IMPLEMENTATION(CameraComponent)

CameraComponent::CameraComponent(float ar) : RenderableComponent()
, m_aspectRatio(ar)
, m_near(0.1)
, m_far(1000)
, m_fov(60)
, m_left(-20.f)
, m_right(20.f)
, m_top(20.f)
, m_bottom(-20.f)
, m_viewPortMinX(0.f)
, m_viewPortMinY(0.f)
, m_viewPortMaxX(1.f)
, m_viewPortMaxY(1.f)
, m_clearColor(0.f, 0.f, 0.f, 1.f)
, m_projMode(CameraComponent::ProjectionMode::Perspective) {
}

CameraComponent::~CameraComponent() {
}


void CameraComponent::onAttached() {
	__super::onAttached();
	m_owner->getParentScene()->onCameraAdded(m_owner, this);
}

void CameraComponent::onDetached() {
	__super::onDetached();
	m_owner->getParentScene()->onCameraRemoved(m_owner, this);
}


glm::mat4 CameraComponent::viewMatrix() const {
#ifdef _DEBUG
	ASSERT(m_owner != nullptr);
#endif // _DEBUG
	// camera view matrix is inverse of it's world matrix
	// we must to make sure that view matrix is ortho normalize (not scale and perpendicular with each other)
	auto matWorld = m_owner->m_transform.getMatrixWorld();
	auto r = glm::mat3(matWorld);
	auto p = glm::vec3(matWorld[3]);
	if (!IsOrthoNormal(r))
		OrthoNormalize(r);

	r = glm::transpose(r);
	p = -(r * p);
		
	return glm::mat4(r[0].x, r[0].y, r[0].z, 0.f,
		r[1].x, r[1].y, r[1].z, 0.f, 
		r[2].x, r[2].y, r[2].z, 0.f, 
		p.x, p.y, p.z, 1.f);

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


Viewport_t CameraComponent::getViewPort(const glm::vec2& renderSize) const {
	Viewport_t vp;
	m_viewPortMinX = clamp(m_viewPortMinX, 0.f, 1.f);
	m_viewPortMinY = clamp(m_viewPortMinY, 0.f, 1.f);
	m_viewPortMaxX = MIN(MAX(m_viewPortMinX, m_viewPortMaxX), 1.f);
	m_viewPortMaxY = MIN(MAX(m_viewPortMinY, m_viewPortMaxY), 1.f);

	vp.x = m_viewPortMinX * renderSize.x;
	vp.y = m_viewPortMinY * renderSize.y;
	vp.width = m_viewPortMaxX * renderSize.x - vp.x;
	vp.height = m_viewPortMaxY * renderSize.y - vp.y;

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
		vf.points[ViewFrustum_t::PointIndex::RBN] = { xn, -yn, -m_near };

		vf.points[ViewFrustum_t::PointIndex::LTF] = { -xf, yf, -m_far };
		vf.points[ViewFrustum_t::PointIndex::LBF] = { -xf, -yf, -m_far };
		vf.points[ViewFrustum_t::PointIndex::RTF] = { xf, yf, -m_far };
		vf.points[ViewFrustum_t::PointIndex::RBF] = { xf, -yf, -m_far };

	} else {
		ASSERT(false);
	}
	
	return vf;
}


void CameraComponent::render(RenderContext* context) {
	auto renderSz = context->getRenderer()->getRenderSize();
	auto mainCamera = context->getScene()->getMainCamera();
	context->getRenderer()->submitCamera(makeCamera(renderSz), mainCamera == this);
}


Camera_t CameraComponent::makeCamera(const glm::vec2& renderSize) const {
	Camera_t c;
	c.position = getPosition();
	c.lookDirection = getLookDirection();
	c.fov = glm::radians(m_fov);
	c.aspectRatio = m_aspectRatio;
	c.near = m_near;
	c.far = m_far;
	c.backgrounColor = m_clearColor;
	c.viewport = getViewPort(renderSize);
	c.viewFrustum = getViewFrustum();
	c.viewMatrix = viewMatrix();
	c.projMatrix = projectionMatrix();
	
	return c;
}