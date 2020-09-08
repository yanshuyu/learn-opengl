#pragma once
#include"Component.h"
#include<glm/glm.hpp>

class TransformComponent : public Component {
public:
	TransformComponent();
	TransformComponent(const glm::vec3& pos, const glm::vec3& rotate, const glm::vec3& scale);

	static const std::string s_identifier;

	void setPosition(const glm::vec3& pos);
	void setRotation(const glm::vec3& rotate);
	void setScale(const glm::vec3& scale);

	void tanslateBy(const glm::vec3& t);
	void rotateBy(const glm::vec3& r);
	void scaleBy(const glm::vec3& s);

	void applyTransform();
	void resetTransform();

	Component* copy() const override;

	inline std::string indentifier() const override {
		return TransformComponent::s_identifier;
	}

	inline glm::mat4 getMatrix() const {
		return  m_applyedTransform * m_localTransform;
	}

	inline glm::vec3 getPosition() const {
		return m_position;
	}
	
	inline glm::vec3 getRotation() const {
		return m_rotation;
	}

	inline glm::vec3 getScale() const {
		return m_scale;
	}

private:
	void calcTransform();

private:
	glm::vec3 m_position;
	glm::vec3 m_rotation;
	glm::vec3 m_scale;
	glm::mat4 m_localTransform;
	glm::mat4 m_applyedTransform;
};