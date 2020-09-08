#include"TransformComponent.h"
#include<glm/ext/matrix_transform.hpp>


const std::string TransformComponent::s_identifier = "TransformComponent";

TransformComponent::TransformComponent() :TransformComponent(glm::vec3(0), glm::vec3(0), glm::vec3(1)) {
	m_localTransform = glm::mat4(1);
	m_applyedTransform = glm::mat4(1);
}

TransformComponent::TransformComponent(const glm::vec3& pos, const glm::vec3& rotate, const glm::vec3& scale):m_position(pos)
, m_rotation(rotate)
, m_scale(scale) {
	calcTransform();
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

void TransformComponent::tanslateBy(const glm::vec3& t) {
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
	m_applyedTransform *= m_localTransform;
	m_position = glm::vec3(0);
	m_rotation = glm::vec3(0);
	m_scale = glm::vec3(1);
	m_localTransform = glm::mat4(1);
}


void TransformComponent::resetTransform() {
	m_position = glm::vec3(0);
	m_rotation = glm::vec3(0);
	m_scale = glm::vec3(1);
	m_localTransform = glm::mat4(1);
	m_applyedTransform = glm::mat4(1);
}


Component* TransformComponent::copy() const {
	TransformComponent* copyed = new TransformComponent();
	copyed->m_position = m_position;
	copyed->m_rotation = m_rotation;
	copyed->m_scale = m_scale;
	copyed->m_localTransform = m_localTransform;
	copyed->m_applyedTransform = m_applyedTransform;
	return copyed;
}

void TransformComponent::calcTransform() {
	glm::mat4 m(1);
	glm::translate(m, m_position);
	glm::rotate(m, glm::radians(m_rotation.z), glm::vec3(0, 0, 1));
	glm::rotate(m, glm::radians(m_rotation.y), glm::vec3(0, 1, 0));
	glm::rotate(m, glm::radians(m_rotation.x), glm::vec3(1, 0, 0));
	glm::scale(m, m_scale);
	m_localTransform = m;
}
