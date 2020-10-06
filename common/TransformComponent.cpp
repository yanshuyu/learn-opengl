#include"TransformComponent.h"
#include<glm/ext/matrix_transform.hpp>
#include<glm/gtc/quaternion.hpp>
#include<glm/gtx/euler_angles.hpp>
#include"SceneObject.h"


COMPONENT_IDENTIFIER_IMP(TransformComponent, "TransformComponent");

TransformComponent::TransformComponent(SceneObject* owner) :m_position(glm::vec3(0))
, m_rotation(glm::vec3(0))
, m_scale(glm::vec3(1))
, m_rightAxis(Local_Right_Axis)
, m_upAxis(Local_Up_Axis)
, m_forwardAxis(Local_Forward_Axis)
, m_transform(glm::mat4(1.f)) {
	m_owner = owner;
}

TransformComponent::TransformComponent(SceneObject* owner, const glm::vec3& pos, const glm::vec3& rotate, const glm::vec3& scale)
: TransformComponent(owner) {
	m_position = pos;
	m_rotation = rotate;
	m_scale = scale;
	calcTransform();
}

TransformComponent& TransformComponent::operator = (const TransformComponent& other) {
	m_position = other.m_position;
	m_rotation = other.m_rotation;
	m_scale = other.m_rotation;
	m_rightAxis = other.m_rightAxis;
	m_upAxis = other.m_upAxis;
	m_forwardAxis = other.m_forwardAxis;
	m_transform = other.m_transform;
	
	return *this;
}


void TransformComponent::setPosition(const glm::vec3& pos) {
	m_position = pos;
	calcTransform();
}

void TransformComponent::setRotation(const glm::vec3& rotate) {
	m_rotation = rotate;
	calcTransform();
}

void TransformComponent::setScale(const glm::vec3& scale) {
	m_scale = scale;
	calcTransform();
}

void TransformComponent::translateBy(const glm::vec3& t) {
	m_position += t;
	calcTransform();
}

void TransformComponent::rotateBy(const glm::vec3& r) {
	m_rotation += r;
	calcTransform();
}

void TransformComponent::scaleBy(const glm::vec3& s) {
	m_scale += s;
	calcTransform();
}


void TransformComponent::lookAt(const glm::vec3& dir, const glm::vec3& up) {
	glm::quat rotateQuat = glm::quatLookAt(glm::normalize(dir), up);
	glm::vec3 angles = glm::eulerAngles(rotateQuat);
	setRotation({ glm::degrees(angles.x), glm::degrees(angles.y), glm::degrees(angles.z) });
}


//void TransformComponent::applyTransform() {
//	m_applyedTransform = m_transform * m_applyedTransform;
//	m_transform = glm::mat4(1.f);
//	m_position = glm::vec3(0.f);
//	m_rotation = glm::vec3(0.f);
//	m_scale = glm::vec3(1.f);
//}


void TransformComponent::resetTransform() {
	m_position = glm::vec3(0.f);
	m_rotation = glm::vec3(0.f);
	m_scale = glm::vec3(1.f);
	m_rightAxis = Local_Right_Axis;
	m_upAxis = Local_Up_Axis;
	m_forwardAxis = Local_Forward_Axis;
	m_transform = glm::mat4(1.f);
}


Component* TransformComponent::copy() const {
	return nullptr;
}


glm::vec3 TransformComponent::getForward() const {
	return m_forwardAxis;
}

glm::vec3 TransformComponent::getUp() const {
	return m_upAxis;
}

glm::vec3 TransformComponent::getRight() const {
	return m_rightAxis;
}


void TransformComponent::getCartesianAxesLocal(glm::vec3* origin, glm::vec3* xAxis, glm::vec3* yAxis, glm::vec3* zAxis) const {
	if (origin)
		*origin = m_position;
	if (xAxis)
		*xAxis = m_rightAxis;
	if (yAxis)
		*yAxis = m_upAxis;
	if (zAxis)
		*zAxis = m_forwardAxis;
}

void TransformComponent::getCartesianAxesWorld(glm::vec3* origin, glm::vec3* xAxis, glm::vec3* yAxis, glm::vec3* zAxis) const {
	glm::mat4 transform = getParentMatrixRecursive();
	if (origin)
		*origin = transform * glm::vec4(m_position, 1.f);

	if (xAxis || yAxis || zAxis) {
		glm::vec3 right(0);
		glm::vec3 up(0);
		glm::vec3 forward(0);
		right = glm::normalize(transform * glm::vec4(m_rightAxis, 0.f));
		up = glm::normalize(transform * glm::vec4(m_upAxis, 0.f));
		forward = glm::normalize(glm::cross(up, right));
		up = glm::normalize(glm::cross(right, forward));

		if (xAxis)
			*xAxis = right;
		if (yAxis)
			*yAxis = up;
		if (zAxis)
			*zAxis = forward;
	}
}


glm::mat4 TransformComponent::getMatrix() const {
	return   m_transform;
}

glm::mat4 TransformComponent::getMatrixWorld() const {
	return getParentMatrixRecursive() * getMatrix();
}

glm::mat4 TransformComponent::getParentMatrix() const {
#ifdef _DEBUG
	ASSERT(m_owner != nullptr);
#endif // _DEBUG

	SceneObject* parent = m_owner->getParent();
	if (parent)
		return parent->m_transform.getMatrix();
	
	return glm::mat4(1);
}

glm::mat4 TransformComponent::getParentMatrixRecursive() const {
#ifdef _DEBUG
	ASSERT(m_owner != nullptr);
#endif // _DEBUG

	glm::mat4 m(1);
	SceneObject* parent = m_owner->getParent();
	while (parent) {
		m *= parent->m_transform.getMatrix();
		parent = parent->getParent();
	}
	
	return m;
}

void TransformComponent::calcTransform() {
	normalizeRotation();

	glm::mat4 m(1);
	m = glm::translate(m, m_position);
	m = glm::rotate(m, glm::radians(m_rotation.z), World_Z_Axis);
	m = glm::rotate(m, glm::radians(m_rotation.y), World_Y_Axis);
	m = glm::rotate(m, glm::radians(m_rotation.x), World_X_Axis);
	//m = glm::yawPitchRoll(glm::radians(m_rotation.y), glm::radians(m_rotation.x), glm::radians(m_rotation.z)) * m;
	m = glm::scale(m, m_scale);
	m_transform = m;

	updateLocalAxes();
}

void TransformComponent::updateLocalAxes() {
	glm::mat4 m = m_transform;
	m_rightAxis = glm::normalize(m * glm::vec4(World_X_Axis, 0.f));
	m_upAxis = glm::normalize(m * glm::vec4(World_Y_Axis, 0.f));
	m_forwardAxis = glm::normalize(glm::cross(m_upAxis, m_rightAxis));
	m_upAxis = glm::normalize(glm::cross(m_rightAxis, m_forwardAxis));
}

void TransformComponent::normalizeRotation() {
	if (m_rotation.x > 360 || m_rotation.x < -360) {
		m_rotation.x = fmodf(m_rotation.x, 360.f);
	}

	if (m_rotation.y > 360 || m_rotation.y < -360) {
		m_rotation.y = fmodf(m_rotation.y, 360.f);
	}

	if (m_rotation.z > 360 || m_rotation.z < -360) {
		m_rotation.z = fmodf(m_rotation.z, 360.f);
	}
}