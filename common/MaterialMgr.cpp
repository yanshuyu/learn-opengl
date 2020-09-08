#include"MaterialMgr.h"
#include<algorithm>



std::shared_ptr<Material> MaterialManager::newMaterial(const std::string& name) {
	auto m = std::make_shared<Material>(name);
	m_materials.insert({ m->id(), m });
	return m;
}

void MaterialManager::addMaterial(Material* m) {
	if (m_materials.find(m->id()) != m_materials.end())
		m_materials.insert({ m->id(), std::shared_ptr<Material>(m) });
}

void MaterialManager::addMaterial(std::shared_ptr<Material> m) {
	if (m_materials.find(m->id()) != m_materials.end())
		m_materials.insert({ m->id(), m });
}

std::shared_ptr<Material> MaterialManager::getMaterial(ID id) const {
	if (m_materials.find(id) != m_materials.end())
		return m_materials.at(id);
	return nullptr;
}

std::shared_ptr<Material> MaterialManager::getFirstMaterial(const std::string& name) const {
	auto pos = std::find_if(m_materials.begin(), m_materials.end(), [&](MaterialMap::value_type m) {
		return m.second->getName() == name;
	});
	
	if (pos != m_materials.end())
		return pos->second;

	return nullptr;
}

std::vector<std::shared_ptr<Material>> MaterialManager::getAllMaterial(const std::string& name) {
	std::vector<std::shared_ptr<Material>> mtls;
	std::for_each(m_materials.begin(), m_materials.end(), [&](MaterialMap::value_type& m) {
		if (m.second->getName() == name)
			mtls.push_back(m.second);
	});
	return mtls;
}

std::shared_ptr<Material> MaterialManager::removeMaterial(ID id) {
	auto pos = m_materials.find(id);
	if (pos != m_materials.end()) {
		m_materials.erase(pos);
		return pos->second;
	}

	return nullptr;
}

std::shared_ptr<Material> MaterialManager::removeFirstMaterial(const std::string& name) {
	auto pos = std::find_if(m_materials.begin(), m_materials.end(), [&](MaterialMap::value_type m) {
		return m.second->getName() == name;
	});

	if (pos != m_materials.end()) {
		m_materials.erase(pos);
		return pos->second;
	}
	
	return  nullptr;
}


size_t MaterialManager::removeAllMaterial(const std::string& name) {
	//auto pos = std::remove_if(m_materials.begin(), m_materials.end(), [&](MaterialMap::value_type m) {
	//	return m.second->getName() == name;
	//});

	//size_t count = std::distance(pos, m_materials.end());
	//m_materials.erase(pos, m_materials.end());
	//
	//return count;

	size_t count = 0;
	for (auto m = m_materials.begin(); m != m_materials.end(); ) {
		if (m->second && m->second->getName() == name) {
			m = m_materials.erase(m);
			count++;
		} else {
			m++;
		}
	}

	return count;
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
	}
	return &defaultMaterial;
}