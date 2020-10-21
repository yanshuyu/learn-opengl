#include"ConditionVariable.h"


template TConditionVariable<int>;
template TConditionVariable<float>;
template TConditionVariable<bool>;


ConditionVariable::ConditionVariable(): ConditionVariable("", Type::Unknown) {

}


ConditionVariable::ConditionVariable(const std::string& name, Type type): m_name(name),
m_type(type) {

}


template<>
TConditionVariable<int>::TConditionVariable(const std::string& name, int val): ConditionVariable(name, Type::Int) {
	m_vaule = val;
}


template<>
TConditionVariable<float>::TConditionVariable(const std::string& name, float val): ConditionVariable(name, Type::Float) {
	m_vaule = val;
}


template<>
TConditionVariable<bool>::TConditionVariable(const std::string& name, bool val): ConditionVariable(name, Type::Bool) {
	m_vaule = val;
}



bool operator == (const ConditionVariable& lhs, const ConditionVariable& rhs) {
	return lhs.m_name == rhs.m_name && lhs.m_type == rhs.m_type;
}

bool operator != (const ConditionVariable& lhs, const ConditionVariable& rhs) {
	return !(lhs == rhs);
}