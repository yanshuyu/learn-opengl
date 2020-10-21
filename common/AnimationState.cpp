#include"AnimationState.h"
#include"AnimationClip.h"
#include"Skeleton.h"

template void AnimationState::addTransition(AnimationState* dst, std::weak_ptr<IntConditionVar> conVar, ConditionComparer cmp, int refVar);
template void AnimationState::addTransition(AnimationState* dst, std::weak_ptr<FloatConditionVar> conVar, ConditionComparer cmp, float refVar);
template void AnimationState::addTransition(AnimationState* dst, std::weak_ptr<BoolConditionVar> conVar, ConditionComparer cmp, bool refVar);


AnimationState::AnimationState(const std::string& name) : m_name(name)
, m_clip()
, m_loopMode(LoopType::Loop)
, m_timer()
, m_progress() {

}

void AnimationState::onEnter() {
	m_timer = 0.f;
	m_progress = 0.f;
}

void AnimationState::onExit() {
	m_timer = 0.f;
	m_progress = 0.f;
}

float AnimationState::update(Pose& outPose, float dt) {
	if (m_clip) {
		m_progress = m_clip->sample(outPose, m_timer, m_loopMode);
		m_timer += dt;
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

void AnimationState::addTransition(AnimationState* dst) {
	if (getTransition(dst))
		return;

	m_transitions.emplace_back(new AnimationTransition(this, dst));
}

template<typename T>
void AnimationState::addTransition(AnimationState* dst, std::weak_ptr<TConditionVariable<T>> conVar, ConditionComparer cmp, T refVar) {
	auto t = getTransition(dst);
	if (!t) {
		addTransition(dst);
		t = m_transitions[m_transitions.size() - 1].get();
	}
	t->addCondition(conVar, cmp, refVar);
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


