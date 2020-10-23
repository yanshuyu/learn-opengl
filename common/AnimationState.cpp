#include"AnimationState.h"
#include"Skeleton.h"



template void AnimationState::addTransition(AnimationState* dst, std::weak_ptr<IntConditionVar> conVar, ConditionComparer cmp, int refVar, float duration);
template void AnimationState::addTransition(AnimationState* dst, std::weak_ptr<FloatConditionVar> conVar, ConditionComparer cmp, float refVar, float duration);
template void AnimationState::addTransition(AnimationState* dst, std::weak_ptr<BoolConditionVar> conVar, ConditionComparer cmp, bool refVar, float duration);


AnimationState::AnimationState(const std::string& name) : m_name(name)
, m_clip()
, m_refPose()
, m_loopMode(LoopType::Loop)
, m_progress()
, m_speed(1.f) {

}

void AnimationState::onEnter() {
	m_progress = 0.f;
}

void AnimationState::onExit() {
	m_progress = 0.f;
}

float AnimationState::update(Pose& outPose, float dt) {
	if (m_clip) {
		m_progress = m_clip->sample(m_refPose, outPose, dt, m_loopMode);
	}
	
	return m_progress;
}

AnimationTransition* AnimationState::firstActiveTransition() const {
	for (size_t i = 0; i < m_transitions.size(); i++)	{
		if (m_transitions[i]->canTransition())
			return m_transitions[i].get();
	}

	return nullptr;
}

void AnimationState::addTransition(AnimationState* dst, float duration) {
	if (getTransition(dst))
		return;

	m_transitions.emplace_back(new AnimationTransition(this, dst));
	m_transitions.back()->setDuration(duration);
}

template<typename T>
void AnimationState::addTransition(AnimationState* dst, std::weak_ptr<TConditionVariable<T>> conVar, ConditionComparer cmp, T refVar, float duration) {
	auto t = getTransition(dst);
	if (!t) {
		addTransition(dst);
		t = m_transitions[m_transitions.size() - 1].get();
	}
	t->addCondition(conVar, cmp, refVar);
	t->setDuration(duration);
}


bool AnimationState::removeTransition(AnimationState* dst) {
	return removeTransition(dst->getName());
}

bool AnimationState::removeTransition(const std::string& dstStateName) {
	int idx = getTransitionIdx(dstStateName);
	if (idx < 0)
		return false;

	m_transitions.erase(m_transitions.begin() + idx);

	return true;
}

AnimationTransition* AnimationState::getTransition(AnimationState* dst) {
	return getTransition(dst->getName());
}

AnimationTransition* AnimationState::getTransition(const std::string& dstStateName) {
	int idx = getTransitionIdx(dstStateName);
	return idx < 0 ? nullptr : m_transitions[idx].get();
}


int AnimationState::getTransitionIdx(const std::string& dstName) const {
	int idx = -1;
	for (size_t i = 0; i < m_transitions.size(); i++) {
		if (m_transitions[i]->getDestinationState()->getName() == dstName) {
			idx = i;
			break;
		}
	}

	return idx;
}


