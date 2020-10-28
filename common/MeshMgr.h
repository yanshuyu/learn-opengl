#pragma once
#include"Singleton.h"
#include"Model.h"
#include"MeshLoader.h"
#include<unordered_map>
#include<string>
#include<memory>
#include<vector>


class MeshManager : public Singleton<MeshManager> {
	typedef std::unordered_map<std::string, std::shared_ptr<Model>> MeshContainer;
public:
	MeshManager() = default;
	~MeshManager() {};

	std::weak_ptr<Model> addMesh(const std::string& fileName,
		int loadingOptions = MeshLoader::Option::LoadAnimations | MeshLoader::Option::LoadMaterials,
		MeshLoader::Preset preset = MeshLoader::Preset::Quality, 
		std::string name = "");
	bool addMesh(Model* mesh);

	std::weak_ptr<Model> createGrid(float width, float depth, float spacing = 1);
	std::weak_ptr<Model> createPlane(float width, float depth);
	std::weak_ptr<Model> createCube();

	std::weak_ptr<Model> getMesh(ID id) const;
	std::weak_ptr<Model> getMesh(const std::string& name)const;

	bool removeMesh(ID id);
	bool removeMesh(const std::string& name);
	
	void removeAllMesh();


protected:
	inline std::string getResourcePath(const std::string& fileName) const;

private:
	std::unordered_map<std::string, std::shared_ptr<Model>> m_meshes;
};
