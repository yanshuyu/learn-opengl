#pragma once
#include"Util.h"
#include<string>

class RTTI {
public:
	virtual ID typeId() const = 0;
	virtual std::string typeName() const = 0;

	virtual bool isType(const ID id) const {
		return false;
	}

	virtual bool isType(const std::string& name) const {
		return false;
	}

	template<typename T>
	T* asType() {
		if (isType(T::s_typeId))
			return static_cast<T*>(this);
		else
			return nullptr;
	}

	template<typename T>
	const T* asType() const {
		if (isType(T::s_typeId))
			return static_cast<const T*>(this);
		else
			return nullptr;
	}
};


#define RTTI_DECLARATION(Type) \
public: \
	static const ID s_typeId; \
	static const std::string s_typeName; \
\
	virtual ID typeId() const override { return s_typeId; } \
	virtual std::string typeName() const override { return s_typeName; } \
	virtual bool isType(const ID id) const override { \
		if (s_typeId == id) \
			return true; \
		else \
			return __super::isType(id); \
	} \
	virtual bool isType(const std::string& name) const override { \
		if (s_typeName == name) \
			return true; \
		else \
			return __super::isType(name); \
	}


#define RTTI_IMPLEMENTATION(Type) \
const ID Type::s_typeId = reinterpret_cast<ID>(&Type::s_typeId); \
const std::string Type::s_typeName = std::string(#Type);