#include"TransitionCondition.h"
#include"Exceptions.h"

template TTransitionCondition<int>;
template TTransitionCondition<float>;
template TTransitionCondition<bool>;

template<typename T>
TTransitionCondition<T>::TTransitionCondition(std::weak_ptr<TConditionVariable<T>> conVar, T refVal, ConditionComparer cmp)
: m_refVal(refVal) {
	m_condVar = conVar;
	m_comparer = cmp;
}

/*
template<typename T>
bool TTransitionCondition<T>::operator == (const TTransitionCondition<T>& other) {
	if (m_condVar.expired() || other.m_condVar.expired())
		return false;

	auto thisVar = m_condVar.lock();
	auto otherVar = other.m_condVar.lock();

	return thisVar->m_name == otherVar->m_name
		&& m_comparer == other.m_comparer
		&& m_refVal == other.m_refVal;
}


template<typename T>
bool TTransitionCondition<T>::operator != (const TTransitionCondition<T>& other) {
	return !(*this == other);
}
*/

template<typename T>
bool TTransitionCondition<T>::satisfied() const {
	if (m_comparer == ConditionComparer::None)
		throw AppException(AppException::Error::ANIMATOR_CONDCMP_NO_COMPARER, "Condition variable comparision no comparer");

	if (m_condVar.expired()) // condition var was removed by animator
		throw AppException(AppException::Error::ANIMATOR_CONCMP_BAD_CONVAR, "Condition variable comparision bad variable");

	auto strongCondVar = m_condVar.lock();
	T condVal = static_cast<TConditionVariable<T>*>(strongCondVar.get())->m_vaule;

	switch (m_comparer)
	{
	case ConditionComparer::Greater:
		return condVal > m_refVal;

	case ConditionComparer::Greater_Equal:
		return condVal >= m_refVal;

	case ConditionComparer::Less:
		return condVal < m_refVal;;

	case ConditionComparer::Less_Equal:
		return condVal <= m_refVal;

	case ConditionComparer::Equal:
		return condVal == m_refVal;

	case ConditionComparer::Not_Equal:
		return condVal != m_refVal;;
	
	default:
		return false;
	}
}


bool operator == (const TransitionCondition& lhs, const TransitionCondition& rhs) {
	if (lhs.m_condVar.expired() || rhs.m_condVar.expired())
		return false;

	auto lVar = lhs.m_condVar.lock();
	auto rVar = rhs.m_condVar.lock();

	return *lVar == *rVar && lhs.m_comparer == rhs.m_comparer;		
}


bool operator != (const TransitionCondition& lhs, const TransitionCondition& rhs) {
	return !(lhs == rhs);
}