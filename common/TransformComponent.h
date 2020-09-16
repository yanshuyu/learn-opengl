#pragma once
#include"Component.h"
#include<glm/glm.hpp>


#define World_X_Axis glm::vec3(1.f, 0.f, 0.f)
#define World_Y_Axis glm::vec3(0.f, 1.f, 0.f)
#define World_Z_Axis glm::vec3(0.f, 0.f, 1.f)
#define Local_Right_Axis glm::vec3(1.f, 0.f, 0.f)
#define Local_Up_Axis glm::vec3(0.f, 1.f, 0.f)
#define Local_Forward_Axis glm::vec3(0.f, 0.f, -1.f)

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

	inline std::string identifier() const override {
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

	//void applyRotation();
	void applyTransform();
	void resetTransform();

	//
	// cartesian coordinator system
	//
	glm::vec3 getForward() const;
	glm::vec3 getUp() const;
	glm::vec3 getRight() const;
	
	void getCartesianAxesLocal(glm::vec3* origin, glm::vec3* xAxis, glm::vec3* yAxis, glm::vec3* zAxis) const;
	void getCartesianAxesWorld(glm::vec3* origin, glm::vec3* xAxis, glm::vec3* yAxis, glm::vec3* zAxis) const;

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
	void updateLocalAxes();
	void normalizeRotation();

private:
	glm::vec3 m_position;
	glm::vec3 m_rotation;
	glm::vec3 m_scale;
	
	glm::vec3 m_rightAxis;
	glm::vec3 m_upAxis;
	glm::vec3 m_forwardAxis;
	
	glm::mat4 m_transform;
	glm::mat4 m_applyedTransform;
};