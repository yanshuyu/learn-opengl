#pragma once
#include"Component.h"
#include<glm/glm.hpp>


class ArcballCameraController : public Component {
public:
	static const std::string s_identifier;

	static ArcballCameraController* create();
	static void destory(ArcballCameraController* c);

	void update(double dt) override;
	std::string identifier() const override;
	Component* copy() const override;

	glm::vec3 cartesianToSpherical(const glm::vec3& p);
	glm::vec3 sphericalToCartesian(const glm::vec3& p);

	inline void setSpeed(float s) {
		m_moveSpeed = s;
	}

	inline void setRadius(float r) {
		m_radius = r;
	}

private:
	float m_moveSpeed = 0.5f;
	float m_lastMouseXPos = 0.f;
	float m_lastMouseYPos = 0.f;
	float m_phi = 0.f;
	float m_theta = 0.f;
	float m_radius = 40;

};