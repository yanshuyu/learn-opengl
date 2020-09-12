#pragma once
#include"Mesh.h"
#include"Material.h"
#include<assimp/Importer.hpp>
#include<assimp/mesh.h>
#include<unordered_map>



class MeshManager;


class MeshGroup {
	friend class MeshManager;
	typedef std::vector<std::unique_ptr<Mesh>> MeshVector;
public:
	MeshGroup();
	MeshGroup(MeshGroup&& other) noexcept;
	MeshGroup& operator = (MeshGroup&& rv) noexcept;
	~MeshGroup() {}

	MeshGroup(const MeshGroup& other) = delete;
	MeshGroup& operator = (const MeshGroup& other) = delete;

public:
	void addMesh(const aiScene* scene, const aiMesh* mesh, aiMatrix4x4 transform, MeshLoadOption options);
	void addMesh(Mesh* mesh);
	void addMesh(std::unique_ptr<Mesh>&& mesh);
	Mesh* addMesh(const std::vector<Vertex_t>& vertices, const std::vector<Index_t>& indices, PrimitiveType pt = PrimitiveType::Triangle);
	Mesh* addMesh(std::vector<Vertex_t>&& vertices, std::vector<Index_t>&& indices, PrimitiveType pt = PrimitiveType::Triangle);

	std::shared_ptr<Material> embededMaterialForMesh(const Mesh* mesh);
	std::shared_ptr<Material> embededMaterialForMesh(ID meshId);

	inline ID id() const {
		return m_id;
	}

	inline std::string filePath() const {
		return m_file;
	}

	inline size_t meshesCount() const {
		return m_meshes.size();
	}

	inline const Mesh* meshAt(size_t idx) const {
		return m_meshes.at(idx).get();
	}

	inline const MeshVector& meshes() const {
		return m_meshes;
	}

	inline std::string getName() const {
		return m_name;
	}

	inline void setName(const std::string& name) {
		m_name = name;
	}

	inline void clear() {
		m_meshes.clear();
	}

	inline size_t embededMaterialCount() const {
		return m_embededMaterials.size();
	} 

private:
	void loadGeometry(const aiMesh* aMesh, std::vector<Vertex_t>& vertices, std::vector<Index_t>& indices, PrimitiveType& pt);
	std::shared_ptr<Material> loadMaterial(const aiScene* aScene, const aiMesh* aMesh);

private:
	std::vector<std::unique_ptr<Mesh>> m_meshes;
	std::unordered_map<ID, std::string> m_embededMaterials;

	std::string m_file;
	std::string m_name;
	ID m_id;
};


std::ostream& operator << (std::ostream& o, const MeshGroup& meshGrp);