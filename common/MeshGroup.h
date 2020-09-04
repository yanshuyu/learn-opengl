#pragma once
#include"Mesh.h"
#include<assimp/Importer.hpp>
#include<assimp/mesh.h>

class MeshManager;


class MeshGroup {
	friend class MeshManager;
	typedef std::vector<std::unique_ptr<Mesh>> MeshContainer;
public:
	MeshGroup();
	MeshGroup(MeshGroup&& other) noexcept;
	MeshGroup& operator = (MeshGroup&& rv) noexcept;
	~MeshGroup() {}

	MeshGroup(const MeshGroup& other) = delete;
	MeshGroup& operator = (const MeshGroup& other) = delete;

public:
	void addMesh(const aiMesh* mesh, aiMatrix4x4 transform);
	void addMesh(Mesh* mesh);
	void addMesh(std::unique_ptr<Mesh>&& mesh);
	Mesh* addMesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, Mesh::PrimitiveType pt = Mesh::PrimitiveType::Triangle);
	Mesh* addMesh(std::vector<Vertex>&& vertices, std::vector<Index>&& indices, Mesh::PrimitiveType pt = Mesh::PrimitiveType::Triangle);


	inline unsigned long id() const {
		return m_id;
	}

	inline std::string filePath() const {
		return m_file;
	}

	inline size_t meshesCount() const {
		return m_meshes.size();
	}

	inline std::string getName() const {
		return m_name;
	}

	inline void setName(const std::string& name) {
		m_name = name;
	}

	inline const Mesh* meshAt(size_t idx) const {
		return m_meshes.at(idx).get();
	}

	//inline MeshContainer::iterator begin() {
	//	return m_meshes.begin();
	//}

	//inline MeshContainer::const_iterator cbegin() const {
	//	return m_meshes.cbegin();
	//}

	//inline MeshContainer::iterator end() {
	//	return m_meshes.end();
	//}

	//inline MeshContainer::const_iterator cend() const {
	//	return m_meshes.cend();
	//}

	inline const MeshContainer& meshes() const {
		return m_meshes;
	}

	inline void clear() {
		m_meshes.clear();
	}

private:
	std::vector<std::unique_ptr<Mesh>> m_meshes;
	std::string m_file;
	std::string m_name;
	unsigned long m_id;
};


std::ostream& operator << (std::ostream& o, const MeshGroup& meshGrp);