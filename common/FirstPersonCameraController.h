#pragma once
#include<common/CameraComponent.h>


class FirstPersonCameraController : public Component {
public:
	static const std::string s_identifier;

	static FirstPersonCameraController* create();
	static void destory(FirstPersonCameraController* c);

	void update(double dt) override;
	std::string indentifier() const override;
	Component* copy() const override;

	inline void setSpeed(float s) {
		m_moveSpeed = s;
	}

private:
	float m_moveSpeed = 8.f;
	float m_lastMouseXPos = 0.f;
	float m_lastMouseYPos = 0.f;
};