#include"MaterialMgr.h"
#include"PhongMaterial.h"
#include"PBRMaterial.h"
#include<algorithm>



std::weak_ptr<IMaterial> MaterialManager::addMaterial(const std::string& name, MaterialType type) {
	auto found = m_materials.find(name);
	if (found != m_materials.end()) {
		return found->second->getType() == type ? found->second : std::shared_ptr<IMaterial>(nullptr);
	}

	m_materials[name] = type == MaterialType::Phong ? 
		std::shared_ptr<IMaterial>(new PhongMaterial(name)) : std::shared_ptr<IMaterial>(new PBRMaterial(name));
	
	return m_materials.at(name);
}

bool MaterialManager::addMaterial(IMaterial* mtl) {
	if (m_materials.find(mtl->getName()) == m_materials.end()) {
		m_materials[mtl->getName()] = std::shared_ptr<IMaterial>(mtl);
		return true;
	}

	return false;
}


std::weak_ptr<IMaterial> MaterialManager::getMaterial(const std::string& name) const {
	if (m_materials.find(name) != m_materials.end())
		return std::weak_ptr<IMaterial>(m_materials.at(name));
	
	return std::weak_ptr<IMaterial>();
}


bool MaterialManager::removeMaterial(const std::string& name) {
	auto pos = m_materials.find(name);
	if (pos != m_materials.end()) {
		m_materials.erase(pos);
		return true;
	}

	return false;
}


void MaterialManager::clearMaterials() {
	m_materials.clear();
}


IMaterial* MaterialManager::defaultPhongMaterial() {
	static PhongMaterial mtl("phong_mtl_default");
	mtl.m_mainColor = glm::vec3(0.7f, 0.7f, 0.7f);
	mtl.m_specularColor = glm::vec3(1.f, 1.f, 1.f);
	mtl.m_emissive = 0.f;
	mtl.m_shininess = 0.5f;
	mtl.m_opacity = 1.f;

	return &mtl;
}


IMaterial* MaterialManager::defaultPBRMaterial() {
	static PBRMaterial mtl("pbr_mtl_default");
	mtl.m_mainColor = glm::vec3(0.7f, 0.7f, 0.7f);
	mtl.m_emissive = 1.f;
	mtl.m_metallic = 0.5f;
	mtl.m_roughness = 0.5f;
	mtl.m_opacity = 1.f;

	return &mtl;
}