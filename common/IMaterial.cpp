#include"IMaterial.h"

RTTI_IMPLEMENTATION(IMaterial)

IMaterial::IMaterial(const std::string& name, MaterialType type): m_type(type)
, m_name(name) {
}