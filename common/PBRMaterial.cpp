#include"PBRMaterial.h"

RTTI_IMPLEMENTATION(PBRMaterial)

PBRMaterial::PBRMaterial(const std::string& name) : IMaterial(name, MaterialType::PBR)
, m_albedoMap()
, m_metallicMap()
, m_roughnessMap()
, m_normalMap()
, m_emissiveMap()
, m_aoMap()
, m_mainColor(0.9f)
, m_emissiveColor(0.f)
, m_metallic(0.5f)
, m_roughness(0.5f)
, m_opacity(1.f) {

}