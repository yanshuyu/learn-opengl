#pragma once
#include"IMaterial.h"
#include<glm/glm.hpp>
#include"Texture.h"
#include"Util.h"

class PhongMaterial: public IMaterial {

public:
	PhongMaterial(const std::string& name);
	PhongMaterial(PhongMaterial&& rv) = default;
	PhongMaterial& operator = (PhongMaterial&& rv) = default;
	~PhongMaterial() {}

	PhongMaterial(const PhongMaterial& other) = delete;
	PhongMaterial& operator = (const PhongMaterial& other) = delete;


	RTTI_DECLARATION(PhongMaterial)

	static const int s_maxShininess;

	inline bool hasAlbedoMap() const {
		return !m_albedoMap.expired();
	}

	inline bool hasSpecularMap() const {
		return !m_specularMap.expired();
	}

	inline bool hasNormalMap() const {
		return !m_normalMap.expired();
	}

	inline bool hasAbientOcclusionMap() const {
		return !m_aoMap.expired();
	}

public:
	std::weak_ptr<Texture> m_albedoMap;
	std::weak_ptr<Texture> m_specularMap;
	std::weak_ptr<Texture> m_normalMap;
	std::weak_ptr<Texture> m_aoMap;

	glm::vec3 m_mainColor;
	glm::vec3 m_specularColor;
	
	float m_opacity;
	float m_shininess;
	float m_emissive;
};