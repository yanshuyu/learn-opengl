#pragma once
#include<common/CameraComponent.h>
#include"Util.h"


class FirstPersonCameraController : public Component {

	RTTI_DECLARATION(FirstPersonCameraController)

public:
	static FirstPersonCameraController* create();
	static void destory(FirstPersonCameraController* c);

	void update(double dt) override;
	Component* copy() const override;

	inline void setSpeed(float s) {
		m_moveSpeed = s;
	}

private:
	float m_moveSpeed = 8.f;
	float m_lastMouseXPos = 0.f;
	float m_lastMouseYPos = 0.f;
};