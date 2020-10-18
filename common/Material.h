#pragma once
#include<glm/glm.hpp>
#include"Texture.h"
#include"Util.h"

class Material {
private:
	Material(const Material& other);

public:
	Material(const std::string& name = "New Materiala");
	Material(Material&& rv) noexcept;
	virtual ~Material() {}

	Material& operator = (const Material& other) = delete;
	Material& operator = (Material&& rv) noexcept;

	static const int s_maxShininess;

	Material* copy() const;
	
	inline bool hasDiffuseTexture() const {
		return !m_diffuseMap.expired();
	}

	inline bool hasSpecularTexture() const {
		return !m_specularMap.expired();
	}

	inline bool hasNormalTexture() const {
		return !m_normalMap.expired();
	}

	inline bool hasEmissiveTexture() const {
		return !m_emissiveMap.expired();
	}

	inline ID id() const {
		return m_id;
	}

	inline void setName(const std::string& name) {
		m_name = name;
	}

	inline std::string getName() const {
		return m_name;
	}

public:
	glm::vec3 m_diffuseColor;
	glm::vec3 m_specularColor;
	glm::vec3 m_emissiveColor;
	float m_opacity;
	float m_shininess;
	float m_ambientAbsorb;
	std::weak_ptr<Texture> m_diffuseMap;
	std::weak_ptr<Texture> m_specularMap;
	std::weak_ptr<Texture> m_emissiveMap;
	std::weak_ptr<Texture> m_normalMap;

protected:
	ID m_id;
	std::string m_name;
};