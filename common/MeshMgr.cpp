#include"MeshMgr.h"
#include"Util.h"
#include<fstream>
#include<algorithm>
#include<iostream>
#include<queue>
#include<assimp/Importer.hpp>
#include<assimp/postprocess.h>



std::shared_ptr<MeshGroup> MeshManager::addModel(const std::string& file, MeshLoadOption options, const std::string& name) {
	std::string modelName(name);
	if (modelName.empty())
		if (!ExtractFileNameFromPath(file, modelName))
			modelName = file;

	Assimp::Importer importer;
	auto scene = importer.ReadFile(file, aiProcessPreset_TargetRealtime_Quality);
	if (!scene) {
#ifdef _DEBUG
		std::cerr << "Failed to load model: \"" << file << "\", reason: " << importer.GetErrorString() << std::endl;
#endif // _DEBUG
		return nullptr;
	}

	auto bonesName = gatherBonesName(scene);
	auto rootBone = findBonesRootNode(scene->mRootNode, bonesName);

	auto meshGroup = std::make_shared<MeshGroup>();
	meshGroup->m_file = file;
	meshGroup->m_name = modelName;

	gatherMeshes(scene, scene->mRootNode, meshGroup.get(), aiMatrix4x4(), options);

#ifdef _DEBUG
	std::cout << "Loaded mesh group " << "\"" << meshGroup->m_name << "\": " << *meshGroup.get() << std::endl;
	std::cout << std::endl << std::endl;
	std::cout << "Parsed mesh bones hiearcy: " << std::endl;
	dumpBoneHiearcy(rootBone);
#endif // _DEBUG

	if (meshGroup->meshesCount() <= 0) {
		return nullptr;
	}

	m_meshes.insert({ modelName, meshGroup });

	return meshGroup;
}


bool MeshManager::addMesh(std::shared_ptr<MeshGroup> mesh, const std::string& name) {
	std::string meshName(name);
	if (meshName.empty())
		meshName = mesh->getName();
	
	if (getMesh(meshName) != nullptr)
		return false;

	m_meshes.insert({ meshName, mesh });
	
	return true;
}

std::shared_ptr<MeshGroup> MeshManager::getMesh(ID id) const {
	auto pos = std::find_if(m_meshes.begin(), m_meshes.end(), [=](const MeshContainer::value_type& mesh) {
		if (mesh.second && mesh.second->id() == id)
			return true;
		return false;
		});

	if (pos == m_meshes.end())
		return nullptr;

	return pos->second;
}

std::shared_ptr<MeshGroup> MeshManager::getMesh(const std::string& name) const {
	auto pos = m_meshes.find(name);
	if (pos == m_meshes.end())
		return nullptr;

	return pos->second;
}


std::shared_ptr<MeshGroup> MeshManager::removeMesh(ID id) {
	auto pos = std::find_if(m_meshes.begin(), m_meshes.end(), [=](const MeshContainer::value_type& mesh) {
		if (mesh.second && mesh.second->id() == id)
			return true;
		return false;
		});

	if (pos == m_meshes.end())
		return nullptr;
	
	m_meshes.erase(pos);

	return pos->second;
}

std::shared_ptr<MeshGroup> MeshManager::removeMesh(const std::string& name) {
	auto pos = m_meshes.find(name);
	if (pos == m_meshes.end())
		return nullptr;

	m_meshes.erase(pos);

	return pos->second;
}


void MeshManager::removeAllMesh() {
	m_meshes.clear();
}


void MeshManager::gatherMeshes(const aiScene* scene, const aiNode* node, MeshGroup* meshGroup, aiMatrix4x4 parentTransform, MeshLoadOption options) {
	aiMatrix4x4 transform = node->mTransformation * parentTransform;
	for (size_t i = 0; i < node->mNumMeshes; i++) {
		const aiMesh* mesh = scene->mMeshes[ node->mMeshes[i] ];
		meshGroup->addMesh(scene, mesh, transform, options);
	}
	
	for (size_t j = 0; j < node->mNumChildren; j++) {
		gatherMeshes(scene, node->mChildren[j], meshGroup, transform, options);
	}
}



std::unordered_set<std::string> MeshManager::gatherBonesName(const aiScene* scene) {
	std::unordered_set<std::string> names;
	for (size_t i = 0; i < scene->mNumMeshes; i++) {
		for (size_t j = 0; j < scene->mMeshes[i]->mNumBones; ++j) {
			names.insert(scene->mMeshes[i]->mBones[j]->mName.C_Str());
		}
	}
	return std::move(names);
}


const aiNode* MeshManager::findBonesRootNode(const aiNode* node, const std::unordered_set<std::string> bonesName) {
	if (!node)
		return nullptr;
	
	std::queue<const aiNode*> rows;
	std::queue<size_t> nodesCountInRow;
	rows.push(node);
	nodesCountInRow.push(1);

	while (!nodesCountInRow.empty()) {
		size_t curRownodeCount = nodesCountInRow.front();
		std::vector<const aiNode*> bonesNodeInRow;
		size_t newRowNodeCount = 0;

		while (curRownodeCount--) {
			auto curNode = rows.front();
			if (bonesName.find(curNode->mName.C_Str()) != bonesName.end()) // a bone node in row
				bonesNodeInRow.push_back(curNode);
			
			rows.pop();
			for (size_t i = 0; i < curNode->mNumChildren; i++) {
				rows.push(curNode->mChildren[i]);
				newRowNodeCount++;
			}
		}

		if (bonesNodeInRow.size() == 1) // find only one bone in row
			return bonesNodeInRow.front();

		if (bonesNodeInRow.size() > 1) { // find multiple bones in row
			std::unordered_set<const aiNode*> parents;
			std::for_each(bonesNodeInRow.begin(), bonesNodeInRow.end(), [&](const aiNode* boneNode) {
				if (boneNode->mParent)
					parents.insert(boneNode->mParent);
			});

			while (parents.size() > 1) {
				std::unordered_set<const aiNode*> copy = parents;
				parents.clear();

				std::for_each(copy.begin(), copy.end(), [&](const aiNode* boneNode) {
					if (boneNode->mParent)
						parents.insert(boneNode->mParent);
				});
			}

			ASSERT(parents.size() == 1);
			return *parents.begin();
		}

		nodesCountInRow.pop();
		if (newRowNodeCount > 0)
			nodesCountInRow.push(newRowNodeCount);
	}
	return nullptr;
}



void MeshManager::dumpBoneHiearcy(const aiNode* root, std::string indent) {
	if (root) {
		std::cout << indent << root->mName.C_Str() << "\n";
		for (size_t i = 0; i < root->mNumChildren; ++i) {
			dumpBoneHiearcy(root->mChildren[i], indent + " | ");
		}
	}
}