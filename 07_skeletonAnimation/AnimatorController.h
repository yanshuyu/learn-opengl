#pragma once
#include<common/Util.h>
#include<common/Component.h>

class AnimatorComponent;
class Pose;

class AnimatorController : public Component {
public:

	COMPONENT_IDENTIFIER_DEC;

	bool initialize() override;

	void setSpeed(float speed);
	
	inline std::string identifier() const override {
		return s_identifier;
	}

	inline  Component* copy() const override {
		return nullptr;
	}
	

protected:
	bool setupAnimationStates();

protected:
	AnimatorComponent* m_animator;
};