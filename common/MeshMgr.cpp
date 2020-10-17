#include"MeshMgr.h"
#include"Util.h"
#include<fstream>
#include<sstream>
#include<algorithm>
#include<iostream>
#include<queue>
#include<assimp/Importer.hpp>
#include<assimp/postprocess.h>



std::shared_ptr<Model> MeshManager::addMesh(const std::string& file, int loadingOptions, MeshLoader::Preset preset, const std::string& name) {
	Model* model = MeshLoader::getInstance()->load(file, loadingOptions, preset, name);
	if (!model)
		return nullptr;

	m_meshes.insert({ model->getName(), std::shared_ptr<Model>(model) });

	return  m_meshes[model->getName()];
}

bool MeshManager::addMesh(std::shared_ptr<Model> mesh, const std::string& name) {
	std::string meshName(name);
	if (meshName.empty())
		meshName = mesh->getName();

	if (getMesh(meshName) != nullptr)
		return false;

	m_meshes.insert({ meshName, mesh });

	return true;
}


std::shared_ptr<Model> MeshManager::createGrid(float width, float depth, float spacing) {
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

	
	auto grid = std::make_shared<Model>();
	grid->setName(ss.str());
	grid->addMesh(std::move(vertices), std::move(indices), PrimitiveType::Line);

	addMesh(grid);

	return grid;
}


std::shared_ptr<Model> MeshManager::createPlane(float width, float depth) {
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

	auto plane = std::make_shared<Model>();
	plane->setName(ss.str());
	plane->addMesh(std::move(vertices), std::move(indices), PrimitiveType::Triangle);

	addMesh(plane);

	return plane;
}


std::shared_ptr<Model> MeshManager::createCube() {
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

	auto cube = std::make_shared<Model>();
	cube->setName(cubeName);
	cube->addMesh(std::move(vertices), std::move(indices), PrimitiveType::Triangle);

	addMesh(cube);

	return cube;
}


std::shared_ptr<Model> MeshManager::getMesh(ID id) const {
	auto pos = std::find_if(m_meshes.begin(), m_meshes.end(), [=](const MeshContainer::value_type& mesh) {
		if (mesh.second && mesh.second->id() == id)
			return true;
		return false;
		});

	if (pos == m_meshes.end())
		return nullptr;

	return pos->second;
}

std::shared_ptr<Model> MeshManager::getMesh(const std::string& name) const {
	auto pos = m_meshes.find(name);
	if (pos == m_meshes.end())
		return nullptr;

	return pos->second;
}


std::shared_ptr<Model> MeshManager::removeMesh(ID id) {
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

std::shared_ptr<Model> MeshManager::removeMesh(const std::string& name) {
	auto pos = m_meshes.find(name);
	if (pos == m_meshes.end())
		return nullptr;

	m_meshes.erase(pos);

	return pos->second;
}


void MeshManager::removeAllMesh() {
	m_meshes.clear();
}
