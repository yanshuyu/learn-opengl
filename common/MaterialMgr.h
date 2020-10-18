#pragma once
#include"Material.h"
#include"Singleton.h"
#include<unordered_map>
#include<memory>



class MaterialManager : public Singleton<MaterialManager> {
	typedef std::unordered_map<std::string, std::shared_ptr<Material>> MaterialMap;

public:
	MaterialManager() = default;

	std::weak_ptr<Material> addMaterial(const std::string& name);
	bool addMaterial(Material* m);

	std::weak_ptr<Material> getMaterial(ID id) const;
	std::weak_ptr<Material> getMaterial(const std::string& name) const;
	
	bool removeMaterial(ID id);
	bool removeMaterial(const std::string& name);
	
	void clearMaterials();

	inline size_t materialCount() const {
		return m_materials.size();
	}

	static Material* defaultMaterial();
		
private:
	std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;
};