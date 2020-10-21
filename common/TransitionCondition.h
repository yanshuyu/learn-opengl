#pragma once
#include"ConditionVariable.h"
#include<memory>


// encapsolate a condition variable weak reference, a referene val and a comparetor,
// determinte wheather a transition can perform
class TransitionCondition {
public:
	virtual ~TransitionCondition() {}

	std::weak_ptr<ConditionVariable> m_condVar;
	ConditionComparer m_comparer;
	
	virtual bool satisfied() const  = 0;
	
	inline bool isValid() const {
		return !m_condVar.expired() && m_comparer != ConditionComparer::None;
	}
};


template<typename T>
class TTransitionCondition : public TransitionCondition {
public:
	TTransitionCondition(std::weak_ptr<TConditionVariable<T>> conVar, T refVal, ConditionComparer cmp);
	
	//bool operator == (const TTransitionCondition<T>& other);
	//bool operator != (const TTransitionCondition<T>& other);

	bool satisfied() const override;

public:
	T m_refVal;
};


bool operator == (const TransitionCondition& lhs, const TransitionCondition& rhs);
bool operator != (const TransitionCondition& lhs, const TransitionCondition& rhs);

