#pragma once
#include"IMaterial.h"
#include"Texture.h"
#include<glm/glm.hpp>

class PBRMaterial : public IMaterial {
public:
	PBRMaterial(const std::string& name);
	PBRMaterial(const PBRMaterial& other) = delete;
 	PBRMaterial(PBRMaterial&& rv) = default;
	~PBRMaterial() {}

	PBRMaterial& operator = (const PBRMaterial& other) = delete;
	PBRMaterial& operator = (PBRMaterial&& rv) = default;

	RTTI_DECLARATION(PBRMaterial)

	inline bool hasAlbedoMap() const {
		return !m_albedoMap.expired();
	}

	inline bool hasMetallicMap() const {
		return !m_metallicMap.expired();
	}

	inline bool hasRoughnessMap() const {
		return !m_roughnessMap.expired();
	}

	inline bool hasNormalMap() const {
		return !m_normalMap.expired();
	}

	inline bool hasAmbientOcclusionMap() const {
		return !m_aoMap.expired();
	}

public:
	std::weak_ptr<Texture> m_albedoMap;
	std::weak_ptr<Texture> m_metallicMap;
	std::weak_ptr<Texture> m_roughnessMap;
	std::weak_ptr<Texture> m_normalMap;
	std::weak_ptr<Texture> m_aoMap;

	glm::vec3 m_mainColor;

	float m_metallic;
	float m_roughness;
	float m_opacity;
	float m_emissive;
};