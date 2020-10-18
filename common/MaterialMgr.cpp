#include"MaterialMgr.h"
#include<algorithm>



std::weak_ptr<Material> MaterialManager::addMaterial(const std::string& name) {
	auto m = std::make_shared<Material>(name);
	m_materials.insert({ name, m });
	return std::weak_ptr<Material>(m);
}

bool MaterialManager::addMaterial(Material* m) {
	if (m->getName().empty())
		return false;

	if (m_materials.find(m->getName()) == m_materials.end()) {
		m_materials.insert({ m->getName(), std::shared_ptr<Material>(m) });
		return true;
	}

	return false;
}


std::weak_ptr<Material> MaterialManager::getMaterial(ID id) const {
	auto pos = std::find_if(m_materials.begin(), m_materials.end(), [=](MaterialMap::value_type m) {
		return m.second->id() == id;
	});

	if (pos != m_materials.end())
		return std::weak_ptr<Material>(pos->second);

	return std::weak_ptr<Material>();
}

std::weak_ptr<Material> MaterialManager::getMaterial(const std::string& name) const {
	if (m_materials.find(name) != m_materials.end())
		return std::weak_ptr<Material>(m_materials.at(name));
	
	return std::weak_ptr<Material>();
}


bool MaterialManager::removeMaterial(ID id) {
	auto pos = std::find_if(m_materials.begin(), m_materials.end(), [=](MaterialMap::value_type m) {
		return m.second->id() == id;
		});

	if (pos != m_materials.end()) {
		m_materials.erase(pos);
		return true;
	}

	return  false;

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


Material* MaterialManager::defaultMaterial() {
	static Material defaultMaterial;
	static bool isInitialize = false;
	if (!isInitialize) {
		defaultMaterial.setName("Default Material");
		defaultMaterial.m_diffuseColor = glm::vec3(0.7, 0.7, 0.7);
		isInitialize = true;
	}
	return &defaultMaterial;
}