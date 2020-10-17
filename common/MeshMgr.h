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

	std::shared_ptr<Model> addMesh(const std::string& file,
		int loadingOptions = MeshLoader::Option::LoadAnimations | MeshLoader::Option::LoadMaterials,
		MeshLoader::Preset preset = MeshLoader::Preset::Quality, 
		const std::string& name = "");
	bool addMesh(std::shared_ptr<Model> mesh, const std::string& name = "");

	std::shared_ptr<Model> createGrid(float width, float depth, float spacing = 1);
	std::shared_ptr<Model> createPlane(float width, float depth);
	std::shared_ptr<Model> createCube();

	std::shared_ptr<Model> getMesh(ID id) const;
	std::shared_ptr<Model> getMesh(const std::string& name)const;

	std::shared_ptr<Model> removeMesh(ID id);
	std::shared_ptr<Model> removeMesh(const std::string& name);
	
	void removeAllMesh();

	inline std::string getResourceAbsolutePath() const {
		return "C:/Users/SY/Documents/learn-opengl/res/models/";
	}

	inline std::string getResourceRelativePath() const {
		return "res/models/";
	}

private:
	std::unordered_map<std::string, std::shared_ptr<Model>> m_meshes;
};
