#include"MeshMgr.h"
#include"Util.h"
#include<fstream>
#include<sstream>
#include<algorithm>
#include<iostream>
#include<queue>
#include<assimp/Importer.hpp>
#include<assimp/postprocess.h>



std::shared_ptr<MeshGroup> MeshManager::addModel(const std::string& file, MeshLoadOption options, const std::string& name) {
	std::string modelName(name);
	if (modelName.empty())
		modelName = ExtractFileNameFromPath(file);
	if (modelName.empty())
		modelName = file;

	Assimp::Importer importer;
	auto scene = importer.ReadFile(file, aiProcessPreset_TargetRealtime_Quality);
	if (!scene) {
#ifdef _DEBUG
		std::cerr << "[Mesh Load error] Failed to load model: \"" << file << "\", reason: " << importer.GetErrorString() << std::endl;
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
	std::stringstream msg;
	msg << "Loaded mesh group " << "\"" << meshGroup->m_name << "\": " << *meshGroup.get() << std::endl;
	msg << "Parsed mesh bones hiearcy: " << std::endl;
	CONSOLELOG(msg.str());
	dumpBoneHiearcy(rootBone);
#endif // _DEBUG

	if (meshGroup->meshesCount() <= 0) {
		return nullptr;
	}

	m_meshes.insert({ modelName, meshGroup });

	return meshGroup;
}


std::shared_ptr<MeshGroup> MeshManager::createGrid(float width, float depth, float spacing) {
	std::stringstream ss;
	ss << "Grid@" << width << "x" << depth << "x" << spacing;

	if (auto founded = getMesh(ss.str()))
		return founded;

	std::vector<Vertex_t> vertices;
	std::vector<Index_t> indices;

	size_t row = ceil(depth / spacing);
	size_t col = ceil(width / spacing);
	vertices.reserve((row + col) * 2);
	depth = row * spacing;
	width = col * spacing;

	for (size_t i = 0; i < row; i++) {
		Vertex_t v1, v2;
		v1.position = glm::vec3(-width / 2, 0.f, spacing * i - depth / 2);
		v1.normal = glm::vec3(0.f, 1.f, 0.f);
		v2.position = glm::vec3(width / 2, 0.f, spacing * i - depth / 2);
		v2.normal = glm::vec3(0.f, 1.f, 0.f);
		vertices.push_back(v1);
		vertices.push_back(v2);
	}

	for (size_t j = 0; j < col; j++) {
		Vertex_t v1, v2;
		v1.position = glm::vec3(spacing * j - width / 2, 0, -depth / 2);
		v1.normal = glm::vec3(0, 1, 0);
		v2.position = glm::vec3(spacing * j - width / 2, 0, depth / 2);
		v2.normal = glm::vec3(0, 1, 0);
		vertices.push_back(v1);
		vertices.push_back(v2);
	}

	
	auto grid = std::make_shared<MeshGroup>();
	grid->setName(ss.str());
	grid->addMesh(std::move(vertices), std::move(indices), PrimitiveType::Line);

	addMesh(grid);

	return grid;
}


std::shared_ptr<MeshGroup> MeshManager::createPlane(float width, float depth) {
	std::stringstream ss;
	ss << "Plane" << "@" << width << "x" << depth;

	if (auto founded = getMesh(ss.str()))
		return founded;

	std::vector<Vertex_t> vertices;
	std::vector<Index_t> indices;

	Vertex_t v;
	v.position = { -width * 0.5f, 0.f, depth * 0.5f };
	v.normal = { 0.f, 1.f, 0.f };
	v.uv = { 0.f, 0.f };
	vertices.push_back(v);

	v.position = { width * 0.5f, 0.f, depth * 0.5f };
	v.uv = { 1.0f, 0.f };
	vertices.push_back(v);

	v.position = { width * 0.5f, 0.f, -depth * 0.5f };
	v.uv = { 1.f, 1.f };
	vertices.push_back(v);

	v.position = { -width * 0.5f, 0.f, depth * 0.5f };
	v.uv = { 0.f, 0.f };
	vertices.push_back(v);

	v.position = { width * 0.5f, 0.f, -depth * 0.5f };
	v.uv = { 1.f, 1.f };
	vertices.push_back(v);

	v.position = { -width * 0.5f, 0.f, -depth * 0.5f };
	v.uv = { 0.f, 1.f };
	vertices.push_back(v);

	auto plane = std::make_shared<MeshGroup>();
	plane->setName(ss.str());
	plane->addMesh(std::move(vertices), std::move(indices), PrimitiveType::Triangle);

	addMesh(plane);

	return plane;
}


std::shared_ptr<MeshGroup> MeshManager::createCube() {
	std::string cubeName("Cube");
	
	if (auto founded = getMesh(cubeName))
		return founded;

	std::vector<Vertex_t> vertices;
	Vertex_t v;
	
	// front
	v.position = { -1, 0, 1 };
	v.normal = { 0, 0, 1 };
	v.uv = { 0, 0 };
	vertices.push_back(v);
	
	v.position = { 1, 0, 1 };
	v.uv = { 1, 0 };
	vertices.push_back(v);

	v.position = { 1, 2, 1 };
	v.uv = { 1, 1 };
	vertices.push_back(v);

	v.position = { -1, 2, 1 };
	v.uv = { 0, 1 };
	vertices.push_back(v);

	//top
	v.position = { -1, 2, 1 };
	v.normal = { 0, 1, 0 };
	v.uv = { 0, 0 };
	vertices.push_back(v);

	v.position = { 1, 2, 1 };
	v.uv = { 1, 0 };
	vertices.push_back(v);

	v.position = { 1, 2, -1 };
	v.uv = { 1, 1 };
	vertices.push_back(v);

	v.position = { -1, 2, -1 };
	v.uv = { 0, 1 };
	vertices.push_back(v);

	// back
	v.position = { 1, 0, -1 };
	v.normal = { 0, 0, -1 };
	v.uv = { 0, 0 };
	vertices.push_back(v);

	v.position = { -1, 0, -1 };
	v.uv = { 1, 0 };
	vertices.push_back(v);

	v.position = { -1, 2, -1 };
	v.uv = { 1, 1 };
	vertices.push_back(v);

	v.position = { 1, 2, -1 };
	v.uv = { 0, 1 };
	vertices.push_back(v);


	//bottom
	v.position = { -1, 0, -1 };
	v.normal = { 0, -1, 0 };
	v.uv = { 0, 0 };
	vertices.push_back(v);

	v.position = { 1, 0, -1 };
	v.uv = { 1, 0 };
	vertices.push_back(v);

	v.position = { 1, 0, 1 };
	v.uv = { 1, 1 };
	vertices.push_back(v);

	v.position = { -1, 0, 1 };
	v.uv = { 0, 1 };
	vertices.push_back(v);


	// left
	v.position = { -1, 0, -1 };
	v.normal = { -1, 0, 0 };
	v.uv = { 0, 0 };
	vertices.push_back(v);

	v.position = { -1, 0, 1 };
	v.uv = { 1, 0 };
	vertices.push_back(v);

	v.position = { -1, 2, 1 };
	v.uv = { 1, 1 };
	vertices.push_back(v);

	v.position = { -1, 2, -1 };
	v.uv = { 0, 1 };
	vertices.push_back(v);


	//right
	v.position = { 1, 0, 1 };
	v.normal = { 1, 0, 0 };
	v.uv = { 0, 0 };
	vertices.push_back(v);

	v.position = { 1, 0, -1 };
	v.uv = { 1, 0 };
	vertices.push_back(v);

	v.position = { 1, 2, -1 };
	v.uv = { 1, 1 };
	vertices.push_back(v);

	v.position = { 1, 2, 1 };
	v.uv = { 0, 1 };
	vertices.push_back(v);

	std::vector<Index_t> indices = {
		// front
		0,  1,  2,
		2,  3,  0,
		// top
		4,  5,  6,
		6,  7,  4,
		// back
		8,  9, 10,
		10, 11,  8,
		// bottom
		12, 13, 14,
		14, 15, 12,
		// left
		16, 17, 18,
		18, 19, 16,
		// right
		20, 21, 22,
		22, 23, 20,
	};

	auto cube = std::make_shared<MeshGroup>();
	cube->setName(cubeName);
	cube->addMesh(std::move(vertices), std::move(indices), PrimitiveType::Triangle);

	addMesh(cube);

	return cube;
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
	aiMatrix4x4 transform = parentTransform * node->mTransformation;
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