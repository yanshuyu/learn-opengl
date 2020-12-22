#pragma once
#include"IMaterial.h"
#include"Singleton.h"
#include<unordered_map>
#include<memory>



class MaterialManager : public Singleton<MaterialManager> {
public:
	MaterialManager() = default;

	std::weak_ptr<IMaterial> addMaterial(const std::string& name, MaterialType type);
	
	bool addMaterial(IMaterial* mtl);

	std::weak_ptr<IMaterial> getMaterial(const std::string& name) const;

	bool removeMaterial(const std::string& name);
	
	void clearMaterials();

	inline size_t materialCount() const {
		return m_materials.size();
	}

	static IMaterial* defaultPhongMaterial();
	static IMaterial* defaultPBRMaterial();

private:
	std::unordered_map<std::string, std::shared_ptr<IMaterial>> m_materials;
};