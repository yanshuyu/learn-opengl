#include"AnimatorController.h"
#include<common/SceneObject.h>
#include<common/AnimatorComponent.h>

COMPONENT_IDENTIFIER_IMP(AnimatorController, "AnimatorController");

bool AnimatorController::initialize() { 
	m_animator = m_owner->getComponent<AnimatorComponent>();
	
	ASSERT(m_animator);
	
	return setupAnimationStates();
}


bool AnimatorController::setupAnimationStates() {
	auto weakModel = m_animator->getAvatar();

	if (weakModel.expired())
		return false;

	auto model = weakModel.lock();
	

	auto idle = m_animator->addState("idle");
	auto walk = m_animator->addState("walk");
	auto run = m_animator->addState("run");
	auto speed = m_animator->addConditionVar<float>("speed");

	// states
	idle->setAnimationClip(model->animationAt(4));
	idle->setAnimationLoopType(LoopType::Loop);

	walk->setAnimationClip(model->animationAt(9));
	walk->setAnimationLoopType(LoopType::Loop);

	run->setAnimationClip(model->animationAt(8));
	run->setAnimationLoopType(LoopType::Loop);


	// transitions
	idle->addTransition<float>(walk, speed, ConditionComparer::Greater, 0.f);
	walk->addTransition<float>(idle, speed, ConditionComparer::Less_Equal, 0.f);
	walk->addTransition<float>(run, speed, ConditionComparer::Greater, 2.0f);
	run->addTransition<float>(walk, speed, ConditionComparer::Less_Equal, 2.0f);

}


void AnimatorController::setSpeed(float speed) {
	m_animator->setConditionVar<float>("speed", speed);
}