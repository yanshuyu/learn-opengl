#pragma once
#include"RTTI.h"
#include<string>
#include<glm/glm.hpp>


enum class MaterialType {
	Unknowed,
	Phong,
	PBR,
};


class IMaterial: public RTTI {
public:
	IMaterial(const std::string& name = "", MaterialType type = MaterialType::Unknowed);
	virtual ~IMaterial() {};

	RTTI_DECLARATION(IMaterial)

	inline MaterialType getType() const {
		return m_type;
	}

	inline void setName(const std::string& name) {
		m_name = name;
	}

	inline std::string getName() const {
		return m_name;
	}

protected:
	MaterialType m_type;
	std::string m_name;
};
