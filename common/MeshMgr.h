#pragma once
#include"Singleton.h"
#include"MeshGroup.h"
#include<unordered_map>
#include<unordered_set>
#include<string>
#include<memory>
#include<vector>
#include<assimp/scene.h>


class MeshManager : public Singleton<MeshManager> {
	typedef std::unordered_map<std::string, std::shared_ptr<MeshGroup>> MeshContainer;
public:
	MeshManager() = default;
	~MeshManager() {};

	std::shared_ptr<MeshGroup> addModel(const std::string& file, MeshLoadOption options = MeshLoadOption::None, const std::string& name = "");
	
	std::shared_ptr<MeshGroup> getMesh(ID id) const;
	std::shared_ptr<MeshGroup> getMesh(const std::string& name)const;

	std::shared_ptr<MeshGroup> removeMesh(ID id);
	std::shared_ptr<MeshGroup> removeMesh(const std::string& name);
	void removeAllMesh();

	inline std::string getResourceAbsolutePath() const {
		return "C:/Users/SY/Documents/learn-opengl/res/models/";
	}

	inline std::string getResourceRelativePath() const {
		return "res/models/";
	}

private:
	void gatherMeshes(const aiScene* scene, const aiNode* node, MeshGroup* meshGroup, aiMatrix4x4 parentTransform, MeshLoadOption options);
	std::unordered_set<std::string> gatherBonesName(const aiScene* scene);
	const aiNode* findBonesRootNode(const aiNode* node, const std::unordered_set<std::string> bonesName);
	void dumpBoneHiearcy(const aiNode* root, std::string indent = "");

private:
	std::unordered_map<std::string, std::shared_ptr<MeshGroup>> m_meshes;
};
