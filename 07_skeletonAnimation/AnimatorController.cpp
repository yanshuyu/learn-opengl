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
	auto die = m_animator->addState("die");

	auto speed = m_animator->addConditionVar<float>("speed");
	auto hp = m_animator->addConditionVar<float>("hp", 1.f);
	
	// states
	idle->setAnimationClip(model->animationAt(4));
	idle->setAnimationLoopType(LoopType::Loop);

	walk->setAnimationClip(model->animationAt(9));
	walk->setAnimationLoopType(LoopType::Loop);
	//walk->setAnimationSpeed(2.f);

	run->setAnimationClip(model->animationAt(8));
	run->setAnimationLoopType(LoopType::Loop);

	die->setAnimationClip(model->animationAt(2));
	die->setAnimationLoopType(LoopType::NoLoop);

	//m_animator->anyState()->setAnimationClip(idle->getAnimationClip());
	
	// transitions
	idle->addTransition<float>(walk, speed, ConditionComparer::Greater, 0.f, 2.f);
	walk->addTransition<float>(idle, speed, ConditionComparer::Less_Equal, 0.f, 2.f);
	walk->addTransition<float>(run, speed, ConditionComparer::Greater, 2.0f, 2.f);
	run->addTransition<float>(walk, speed, ConditionComparer::Less_Equal, 2.0f, 2.f);
	m_animator->anyState()->addTransition<float>(die, hp, ConditionComparer::Less_Equal, 0.f, 1.f);
	die->addTransition<float>(idle, hp, ConditionComparer::Greater, 0.f, 1.f);
}


void AnimatorController::setSpeed(float speed) {
	m_animator->setConditionVar<float>("speed", speed);
}

void AnimatorController::setHp(float hp) {
	m_animator->setConditionVar<float>("hp", hp);
}