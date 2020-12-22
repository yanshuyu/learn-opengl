#include"PhongMaterial.h"

RTTI_IMPLEMENTATION(PhongMaterial)

const int PhongMaterial::s_maxShininess = 100;

PhongMaterial::PhongMaterial(const std::string& name) :IMaterial(name, MaterialType::Phong)
, m_mainColor(0.8f, 0.8f, 0.8f)
, m_specularColor(1.f)
, m_emissiveColor(0.f) 
, m_opacity(1.f)
, m_shininess(0.5f)
, m_albedoMap()
, m_specularMap()
, m_emissiveMap()
, m_normalMap()
, m_aoMap() {
	
}
