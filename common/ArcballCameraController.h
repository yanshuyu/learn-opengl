#pragma once
#include"Component.h"
#include<glm/glm.hpp>

class Notification;

class ArcballCameraController : public Component {
public:
	ArcballCameraController();
	~ArcballCameraController();

	static const std::string s_identifier;

	static ArcballCameraController* create();
	static void destory(ArcballCameraController* c);

	std::string identifier() const override;
	Component* copy() const override;
	
	void setPosition(const glm::vec3& pos);
	void update(double dt) override;
	void onMouseScrolling(const Notification* nc);

	inline void setSpeed(float s) {
		m_moveSpeed = s;
	}

	inline void setTarget(const glm::vec3& target) {
		m_target = target;
	}

private:
	glm::vec3 sphericalToCartesian(float radius, float theta, float phi);
	glm::vec3 cartesianToSpherical(const glm::vec3& pos);

private:
	float m_moveSpeed = 2.f;
	float m_lastMouseXPos = 0.f;
	float m_lastMouseYPos = 0.f;
	float m_phi = 0.f;
	float m_theta = 0.f;
	float m_radius = 40;
	glm::vec3 m_target = { 0.f, 0.f, 0.f };
};