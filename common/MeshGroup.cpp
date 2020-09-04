#include"MeshGroup.h"
#include"Util.h"
#include<assimp/material.h>
#include<glm/gtc/type_ptr.hpp>

MeshGroup::MeshGroup() :m_name()
, m_id(0)
, m_file()
, m_meshes() {
	m_id = reinterpret_cast<unsigned long>(this);
}

MeshGroup::MeshGroup(MeshGroup&& rv) noexcept :m_name(std::move(rv.m_name))
, m_id(0)
, m_file(std::move(rv.m_file))
, m_meshes(std::move(rv.m_meshes)) {
	m_id = reinterpret_cast<unsigned long>(this);
}

MeshGroup& MeshGroup::operator = (MeshGroup&& rv) noexcept {
	m_name = std::move(rv.m_name);
	m_file = std::move(rv.m_file);
	m_meshes = std::move(rv.m_meshes);
	return *this;
}

void MeshGroup::addMesh(const aiMesh* mesh, aiMatrix4x4 transform) {
	std::vector<Vertex> vertices;
	vertices.reserve(mesh->mNumVertices);

	for (size_t i = 0; i < mesh->mNumVertices; i++) {
		Vertex v;
		ASSERT(mesh->HasPositions());
		const aiVector3D pos = mesh->mVertices[i];

		ASSERT(mesh->HasNormals());
		const aiVector3D normal = mesh->mNormals[i];

		ASSERT(mesh->HasTangentsAndBitangents());
		const aiVector3D tangent = mesh->mTangents[i];
		const aiVector3D biTangent = mesh->mBitangents[i];

		const aiVector3D uv = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : aiVector3D();

		v.position = glm::vec3(pos.x, pos.y, pos.z);
		v.normal = glm::vec3(normal.x, normal.y, normal.z);
		v.tangent = glm::vec3(tangent.x, tangent.y, tangent.z);
		v.biTangent = glm::vec3(biTangent.x, biTangent.y, biTangent.z);
		v.uv = glm::vec2(uv.x, uv.y);
		vertices.push_back(v);
	}

	if (vertices.empty())
		return;
	
	Mesh::PrimitiveType pt = Mesh::PrimitiveType::Unknown;
	switch (mesh->mPrimitiveTypes)
	{
	case aiPrimitiveType_POINT:
		pt = Mesh::PrimitiveType::Point;
		break;
	case aiPrimitiveType_LINE:
		pt = Mesh::PrimitiveType::Line;
		break;
	case aiPrimitiveType_TRIANGLE:
		pt = Mesh::PrimitiveType::Triangle;
		break;
	case aiPrimitiveType_POLYGON:
		pt = Mesh::PrimitiveType::Polygon;
		break;
	default:
		break;
	}

	std::vector<Index> indices;
	size_t primitiveIndexCount = 3;
	if (pt == Mesh::PrimitiveType::Polygon) {
		primitiveIndexCount = 4;
	}else if (pt == Mesh::PrimitiveType::Point) {
		primitiveIndexCount = 2;
	}else if (pt == Mesh::PrimitiveType::Point) {
		primitiveIndexCount = 1;
	}
	indices.reserve(mesh->mNumFaces * primitiveIndexCount);

	for (size_t j = 0; j < mesh->mNumFaces; j++) {
		const aiFace face = mesh->mFaces[j];
		for (size_t k=0; k < face.mNumIndices; k++) {
			indices.push_back(face.mIndices[k]);
		}
	}

	aiMatrix4x4 colMajorTransform = transform.Transpose();
	Mesh* addedMesh = addMesh(std::move(vertices), std::move(indices), pt);
	addedMesh->m_name = mesh->mName.C_Str();
	addedMesh->m_transform = glm::make_mat4(&colMajorTransform[0][0]);
}

void MeshGroup::addMesh(Mesh* mesh) {
	m_meshes.emplace_back(mesh);
}

void MeshGroup::addMesh(std::unique_ptr<Mesh>&& mesh) {
	m_meshes.push_back(std::move(mesh));
}

Mesh* MeshGroup::addMesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, Mesh::PrimitiveType pt) {
	auto mesh = std::make_unique<Mesh>();
	mesh->fill(vertices, indices, pt);
	m_meshes.push_back(std::move(mesh));
	return m_meshes.back().get();
}

Mesh* MeshGroup::addMesh(std::vector<Vertex>&& vertices, std::vector<Index>&& indices, Mesh::PrimitiveType pt) {
	auto mesh = std::make_unique<Mesh>();
	mesh->fill(std::move(vertices), std::move(indices), pt);
	m_meshes.push_back(std::move(mesh));
	return m_meshes.back().get();
}



std::ostream& operator << (std::ostream& o, const MeshGroup& meshGrp) {
	o << "{ name: " << meshGrp.getName() << "(" << meshGrp.id() << "),\n";
	o << " filePath: " << meshGrp.filePath() << ",\n";
	o << " meshes(cout:" << meshGrp.meshesCount() << ")";
	for (size_t i = 0; i < meshGrp.meshesCount(); i++) {
		o << " " << *meshGrp.meshAt(i) << "\n";
	}
	o << "}";
	return o;
}
