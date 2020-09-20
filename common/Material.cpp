#include"Material.h"

const int Material::s_maxShininess = 100;

Material::Material(const std::string& name) :m_name(name)
, m_id(0)
, m_diffuseColor(1,1,1)
, m_specularColor(1,1,1)
, m_emissiveColor(0,0,0) 
, m_opacity(1)
, m_shininess(0.5)
, m_ambientAbsorb(0)
, m_diffuseMap(nullptr)
, m_normalMap(nullptr)
, m_emissiveMap(nullptr){
	m_id = reinterpret_cast<ID>(this);
}

Material::Material(Material&& rv)noexcept : m_diffuseColor(std::move(rv.m_diffuseColor))
, m_specularColor(std::move(rv.m_specularColor))
, m_emissiveColor(std::move(rv.m_emissiveColor))
, m_opacity(std::move(rv.m_opacity))
, m_shininess(std::move(rv.m_shininess))
, m_ambientAbsorb(std::move(rv.m_ambientAbsorb))
, m_diffuseMap(std::move(rv.m_diffuseMap))
, m_normalMap(std::move(rv.m_normalMap))
, m_emissiveMap(std::move(rv.m_emissiveMap))
, m_id(0){
	m_id = reinterpret_cast<ID>(this);
}


Material::Material(const Material& other) {
	m_diffuseColor = other.m_diffuseColor;
	m_specularColor = other.m_specularColor;
	m_emissiveColor = other.m_emissiveColor;
	m_opacity = other.m_opacity;
	m_shininess = other.m_shininess;
	m_ambientAbsorb = other.m_ambientAbsorb;
	m_diffuseMap = other.m_diffuseMap;
	m_normalMap = other.m_normalMap;
	m_emissiveMap = other.m_emissiveMap;
	m_id = reinterpret_cast<ID>(this);
}


Material& Material::operator = (Material&& rv) noexcept {
	m_diffuseColor = std::move(rv.m_diffuseColor);
	m_specularColor = std::move(rv.m_specularColor);
	m_emissiveColor = std::move(rv.m_emissiveColor);
	m_opacity = std::move(rv.m_opacity);
	m_shininess = std::move(rv.m_shininess);
	m_ambientAbsorb = std::move(rv.m_ambientAbsorb);
	m_diffuseMap = std::move(rv.m_diffuseMap);
	m_normalMap = std::move(rv.m_normalMap);
	m_emissiveMap = std::move(rv.m_emissiveMap);
	
	return *this;
}


Material* Material::copy() const {
	Material* copyed = new Material(*this);
	return copyed;
}