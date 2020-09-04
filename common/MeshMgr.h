#pragma once
#include"Singleton.h"
#include"MeshGroup.h"
#include<unordered_map>
#include<unordered_set>
#include<string>
#include<memory>
#include<vector>
#include<stack>
#include<assimp/scene.h>


class MeshManager : public Singleton<MeshManager> {
	typedef std::unordered_map<int, std::shared_ptr<MeshGroup>> MeshContainer;
public:
	enum MeshLoadOption {
		LoadMaterial = 1 << 0,
		LoadTexture = 1 << 1,
		LoadAnimation = 1 << 2,
	};

	MeshManager() = default;
	~MeshManager() {};

	std::shared_ptr<MeshGroup> load(const std::string& file);
	
	std::shared_ptr<MeshGroup> getMesh(int id) const;
	std::shared_ptr<MeshGroup> getFirstMesh(const std::string& name)const;
	std::vector<std::shared_ptr<MeshGroup>> getMesh(const std::string& name) const;

	std::shared_ptr<MeshGroup> removeMesh(int id);
	std::shared_ptr<MeshGroup> removeFirstMesh(const std::string& name);
	size_t removeMesh(const std::string& name);
	void removeAllMesh();

private:
	void gatherMeshes(const aiScene* scene, const aiNode* node, MeshGroup* meshGroup, aiMatrix4x4 parentTransform);
	std::unordered_set<std::string> gatherBonesName(const aiScene* scene);
	const aiNode* findBonesRootNode(const aiNode* node, const std::unordered_set<std::string> bonesName);
	void dumpBoneHiearcy(const aiNode* root, std::string indent = "");

private:
	std::unordered_map<int, std::shared_ptr<MeshGroup>> m_meshes;
};
