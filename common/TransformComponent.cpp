#include"TransformComponent.h"
#include<glm/ext/matrix_transform.hpp>
#include"SceneObject.h"


const std::string TransformComponent::s_identifier = "TransformComponent";

TransformComponent::TransformComponent(SceneObject* owner) :m_position(glm::vec3(0))
, m_rotation(glm::vec3(0))
, m_scale(glm::vec3(1))
, m_transform(glm::mat4(1))
, m_applyedTransform(glm::mat4(1)) {
	m_owner = owner;
}

TransformComponent::TransformComponent(SceneObject* owner, const glm::vec3& pos, const glm::vec3& rotate, const glm::vec3& scale)
: m_position(pos)
, m_rotation(rotate)
, m_scale(scale) {
	m_owner = owner;
	calcTransform();
}

TransformComponent& TransformComponent::operator = (const TransformComponent& other) {
	m_position = other.m_position;
	m_rotation = other.m_rotation;
	m_scale = other.m_rotation;
	m_transform = other.m_transform;
	m_applyedTransform = other.m_applyedTransform;
	
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

void TransformComponent::applyTransform() {
	m_applyedTransform *= m_transform;
	m_position = glm::vec3(0);
	m_rotation = glm::vec3(0);
	m_scale = glm::vec3(1);
	m_transform = glm::mat4(1);
}


void TransformComponent::resetTransform() {
	m_position = glm::vec3(0);
	m_rotation = glm::vec3(0);
	m_scale = glm::vec3(1);
	m_transform = glm::mat4(1);
	m_applyedTransform = glm::mat4(1);
}


Component* TransformComponent::copy() const {
	return nullptr;
}

void TransformComponent::calcTransform() {
	glm::mat4 m(1);
	m = glm::translate(m, m_position);
	m = glm::rotate(m, glm::radians(m_rotation.z), glm::vec3(0, 0, 1));
	m = glm::rotate(m, glm::radians(m_rotation.y), glm::vec3(0, 1, 0));
	m = glm::rotate(m, glm::radians(m_rotation.x), glm::vec3(1, 0, 0));
	m = glm::scale(m, m_scale);

	m_transform = m;
}

void TransformComponent::getCartesianAxesLocal(glm::vec3& xAxis, glm::vec3& yAxis, glm::vec3& zAxis) {
	calcCartesianAxes( getMatrix(), xAxis, yAxis, zAxis);
}

void TransformComponent::getCartesianAxesWorld(glm::vec3& xAxis, glm::vec3& yAxis, glm::vec3& zAxis) {
	calcCartesianAxes(getMatrixWorld(), xAxis, yAxis, zAxis);
}

void TransformComponent::calcCartesianAxes(glm::mat4 transform, glm::vec3& xAxis, glm::vec3& yAxis, glm::vec3& zAxis) {
	glm::vec3 X(1, 0, 0);
	glm::vec3 Y(0, 1, 0);
	glm::vec3 Z(0, 0, 1);

	X = glm::normalize(transform * glm::vec4(X, 0));
	Y = glm::normalize(transform * glm::vec4(Y, 0));
	Z = glm::normalize(glm::cross(X, Y));
	Y = glm::normalize(glm::cross(Z, X));
	
	xAxis = X;
	yAxis = Y;
	zAxis = Z;
}


glm::mat4 TransformComponent::getMatrix() const {
	return  m_applyedTransform * m_transform;
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