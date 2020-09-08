#pragma once
#include"Material.h"
#include"Singleton.h"
#include<unordered_map>
#include<memory>



class MaterialManager : public Singleton<MaterialManager> {
	typedef std::unordered_map<ID, std::shared_ptr<Material>> MaterialMap;

public:
	MaterialManager() = default;

	std::shared_ptr<Material> newMaterial(const std::string& name = "New Material");
	void addMaterial(Material* m);
	void addMaterial(std::shared_ptr<Material> m);

	std::shared_ptr<Material> getMaterial(ID id) const;
	std::shared_ptr<Material> getFirstMaterial(const std::string& name) const;
	std::vector<std::shared_ptr<Material>> getAllMaterial(const std::string& name);

	std::shared_ptr<Material> removeMaterial(ID id);
	std::shared_ptr<Material> removeFirstMaterial(const std::string& name);
	size_t removeAllMaterial(const std::string& name);
	void clearMaterials();

	inline size_t materialCount() const {
		return m_materials.size();
	}

	static Material* defaultMaterial();
		
private:
	std::unordered_map<ID, std::shared_ptr<Material>> m_materials;
};