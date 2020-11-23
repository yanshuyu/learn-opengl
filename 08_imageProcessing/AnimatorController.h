#pragma once
#include<common/Util.h>
#include<common/Component.h>

class AnimatorComponent;
class Pose;

class AnimatorController : public Component {

	RTTI_DECLARATION(AnimatorController)

public:

	bool initialize() override;

	void setSpeed(float speed);
	void setHp(float hp);
	
	inline  Component* copy() const override {
		return nullptr;
	}
	

protected:
	bool setupAnimationStates();

protected:
	std::weak_ptr<AnimatorComponent> m_animator;
};