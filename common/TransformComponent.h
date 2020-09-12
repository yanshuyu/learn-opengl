#pragma once
#include"Component.h"
#include<glm/glm.hpp>


class SceneObject;
class CameraComponent;


class TransformComponent : public Component {
	friend class SceneObject;
	friend class CameraComponent;

protected:
	TransformComponent(SceneObject* owner);
	TransformComponent(SceneObject* owner, const glm::vec3& pos, const glm::vec3& rotate, const glm::vec3& scale);

public:
	TransformComponent() = delete;
	TransformComponent(const TransformComponent& other) = delete;
	TransformComponent(TransformComponent&& rv) = delete;
	TransformComponent& operator = (TransformComponent&& rv) = delete;
	TransformComponent& operator = (const TransformComponent& other);

	static const std::string s_identifier;

	//
	// inherit override
	//
	Component* copy() const override;

	inline std::string indentifier() const override {
		return TransformComponent::s_identifier;
	}

	//
	// transform operation
	//
	void setPosition(const glm::vec3& pos);
	void setRotation(const glm::vec3& rotate);
	void setScale(const glm::vec3& scale);

	void translateBy(const glm::vec3& t);
	void rotateBy(const glm::vec3& r);
	void scaleBy(const glm::vec3& s);

	void applyTransform();
	void resetTransform();

	//
	// cartesian coordinator system
	//
	void getCartesianAxesLocal(glm::vec3& xAxis, glm::vec3& yAxis, glm::vec3& zAxis);
	void getCartesianAxesWorld(glm::vec3& xAxis, glm::vec3& yAxis, glm::vec3& zAxis);

	//
	// matrix for transform
	//
	glm::mat4 getMatrix() const;
	glm::mat4 getMatrixWorld() const;
	glm::mat4 getParentMatrix() const;
	glm::mat4 getParentMatrixRecursive() const;

	//
	// public getter setter
	//
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
	void calcCartesianAxes(glm::mat4 transform, glm::vec3& xAxis, glm::vec3& yAxis, glm::vec3& zAxis);

private:
	glm::vec3 m_position;
	glm::vec3 m_rotation;
	glm::vec3 m_scale;
	glm::mat4 m_transform;
	glm::mat4 m_applyedTransform;
};