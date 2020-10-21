#pragma once
#include"TransitionCondition.h"
#include<vector>
#include<functional>


class AnimationState;


// transition contain src and dst states and collection of conditions
// transition can perform when all conditions are satisfied,
class AnimationTransition {
	friend bool operator == (const AnimationTransition& lhs, const AnimationTransition& rhs);
	friend bool operator != (const AnimationTransition& lhs, const AnimationTransition& rhs);

public:
	AnimationTransition(AnimationState* src, AnimationState* dst);
	virtual ~AnimationTransition() {}

	AnimationTransition(const AnimationTransition& other) = delete;
	AnimationTransition& operator = (const AnimationTransition& other) = delete;

	template<typename T>
	TTransitionCondition<T>* addCondition(std::weak_ptr<TConditionVariable<T>> conVar, ConditionComparer cmp, T refVar);
	
	template<typename T>
	TTransitionCondition<T>* getCondition(const std::string& conVarName);

	template<typename T>
	TTransitionCondition<T>* getCondition(const std::string& conVarName, ConditionComparer cmp);

	bool removeCondition(const std::string& conVarName);
	bool removeCondition(const std::string& conVarName, ConditionComparer cmp);

	void removeAllInvalidConditions();

	bool canTransition() const;
	
	inline size_t conditionCount() const {
		return m_conditions.size();
	}
	
	inline TransitionCondition* conditionAt(size_t idx) {
		return m_conditions[idx].get();
	}

	inline AnimationState* getSourceState() {
		return m_srcState;
	}

	inline AnimationState* getDestinationState() {
		return m_dstState;
	}

protected:
	int getConditionIndex(std::function<bool(const TransitionCondition*)> criteria);
	int getConditionIndex(const std::string& name);
	int getConditionIndex(const std::string& name, ConditionComparer cmp);

protected:
	AnimationState* m_srcState;
	AnimationState* m_dstState;
	std::vector<std::unique_ptr<TransitionCondition>> m_conditions;
};


bool operator == (const AnimationTransition& lhs, const AnimationTransition& rhs);
bool operator != (const AnimationTransition& lhs, const AnimationTransition& rhs);