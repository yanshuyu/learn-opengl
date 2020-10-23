#include"AnimationTransition.h"
#include"AnimationState.h"
#include"AnimationClip.h"
#include"Util.h"

template TTransitionCondition<int>* AnimationTransition::addCondition(std::weak_ptr<IntConditionVar> conVar, ConditionComparer cmp, int refVar);
template TTransitionCondition<float>* AnimationTransition::addCondition(std::weak_ptr<FloatConditionVar> conVar, ConditionComparer cmp, float refVar);
template TTransitionCondition<bool>* AnimationTransition::addCondition(std::weak_ptr<BoolConditionVar> conVar, ConditionComparer cmp, bool refVar);

template TTransitionCondition<int>* AnimationTransition::getCondition<int>(const std::string& conVarName);
template TTransitionCondition<float>* AnimationTransition::getCondition<float>(const std::string& conVarName);
template TTransitionCondition<bool>* AnimationTransition::getCondition<bool>(const std::string& conVarName);

template TTransitionCondition<int>* AnimationTransition::getCondition<int>(const std::string& conVarName, ConditionComparer cmp);
template TTransitionCondition<float>* AnimationTransition::getCondition<float>(const std::string& conVarName, ConditionComparer cmp);
template TTransitionCondition<bool>* AnimationTransition::getCondition<bool>(const std::string& conVarName, ConditionComparer cmp);



AnimationTransition::AnimationTransition(AnimationState* src, AnimationState* dst) : m_srcState(src)
, m_dstState(dst)
, m_duration(0.f)
, m_conditions() {

}

template<typename T>
TTransitionCondition<T>* AnimationTransition::addCondition(std::weak_ptr<TConditionVariable<T>> conVar, ConditionComparer cmp, T refVar) {
	if (!conVar.expired()) {
		int idx = getConditionIndex(conVar.lock()->m_name, cmp);
		if (idx >= 0) {
			return static_cast<TTransitionCondition<T>*>(m_conditions[idx].get());
		}
	}

	m_conditions.emplace_back(new TTransitionCondition<T>(conVar, refVar, cmp));
	
	return static_cast<TTransitionCondition<T>*>(m_conditions[m_conditions.size() - 1].get());
}

template<typename T>
TTransitionCondition<T>* AnimationTransition::getCondition(const std::string& conVarName) {
	int idx = getConditionIndex(conVarName);
	if (idx < 0)
		return nullptr;

	return static_cast<TTransitionCondition<T>*>(m_conditions[idx].get());
}


template<typename T>
TTransitionCondition<T>* AnimationTransition::getCondition(const std::string& conVarName, ConditionComparer cmp) {
	int idx = getConditionIndex(conVarName, cmp);
	if (idx < 0)
		return nullptr;

	return static_cast<TTransitionCondition<T>*>(m_conditions[idx].get());
}


bool AnimationTransition::removeCondition(const std::string& varName) {
	int idx = getConditionIndex(varName);
	if (idx < 0)
		return false;

	m_conditions.erase(m_conditions.begin() + idx);

	return true;
}


bool AnimationTransition::removeCondition(const std::string& conVarName, ConditionComparer cmp) {
	int idx = getConditionIndex(conVarName, cmp);
	if (idx < 0)
		return false;

	m_conditions.erase(m_conditions.begin() + idx);

	return true;
}


void AnimationTransition::removeAllInvalidConditions() {
	for (auto itr = m_conditions.begin(); itr != m_conditions.end(); ) {
		if (!(*itr)->isValid()) {
			itr = m_conditions.erase(itr);
		}
		else {
			itr++;
		}
	}
}


bool AnimationTransition::canTransition() const {
	bool canTransition = true;
	for (auto& cond : m_conditions) {
		if (!cond->satisfied()) {
			canTransition = false;
		}
	}

	return canTransition;
}


void AnimationTransition::setDuration(float dur) {
	float minLen = MIN(m_srcState->getAnimationDuration(), m_dstState->getAnimationDuration());
	m_duration = MIN(dur, minLen);
}


int AnimationTransition::getConditionIndex(std::function<bool(const TransitionCondition*)> criteria) {
	int idx = 0;
	bool found = false;
	for (auto& con : m_conditions) {
		if (criteria(con.get())) {
			found = true;
			break;
		}
		idx++;
	}

	return found ? idx : -1;
}


int AnimationTransition::getConditionIndex(const std::string& name) {
	return getConditionIndex([&](const TransitionCondition* cond) {
		if (!cond->m_condVar.expired())
			return cond->m_condVar.lock()->m_name == name;
		return false;
	});
}


int AnimationTransition::getConditionIndex(const std::string& name, ConditionComparer cmp) {
	return getConditionIndex([&](const TransitionCondition* cond) {
		if (!cond->m_condVar.expired())
			return cond->m_condVar.lock()->m_name == name && cond->m_comparer == cmp;
		return false;
	});
}



bool operator == (const AnimationTransition& lhs, const AnimationTransition& rhs) {
	return lhs.m_srcState == rhs.m_srcState && lhs.m_dstState == rhs.m_dstState;
}

bool operator != (const AnimationTransition& lhs, const AnimationTransition& rhs) {
	return !(lhs == rhs);
}


