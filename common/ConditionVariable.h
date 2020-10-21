#pragma once
#include<string>

enum class ConditionComparer {
	None,
	Greater,
	Greater_Equal,
	Less,
	Less_Equal,
	Equal,
	Not_Equal,
};



class ConditionVariable {
public:
	enum class Type {
		Unknown,
		Int,
		Float,
		Bool,
	};
public:
	ConditionVariable();
	ConditionVariable(const std::string& name, Type type);
	virtual ~ConditionVariable() {}

	Type m_type;
	std::string m_name;
};



template<typename T>
class TConditionVariable : public ConditionVariable {
public:
	TConditionVariable(const std::string& name, T val = T());
	T m_vaule;
};


bool operator == (const ConditionVariable& lhs, const ConditionVariable& rhs);
bool operator != (const ConditionVariable& lhs, const ConditionVariable& rhs);


typedef TConditionVariable<int> IntConditionVar;
typedef TConditionVariable<float> FloatConditionVar;
typedef TConditionVariable<bool> BoolConditionVar;
