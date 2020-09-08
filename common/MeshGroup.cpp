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
	std::vector<Vertex_t> vertices;
	vertices.reserve(mesh->mNumVertices);

	for (size_t i = 0; i < mesh->mNumVertices; i++) {
		Vertex_t v;
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
	
	PrimitiveType pt = PrimitiveType::Unknown;
	size_t primitiveIndexCount = 3;
	switch (mesh->mPrimitiveTypes)
	{
	case aiPrimitiveType_POINT:
		pt = PrimitiveType::Point;
		primitiveIndexCount = 1;
		break;
	case aiPrimitiveType_LINE:
		pt = PrimitiveType::Line;
		primitiveIndexCount = 2;
		break;
	case aiPrimitiveType_TRIANGLE:
		pt = PrimitiveType::Triangle;
		primitiveIndexCount = 3;
		break;
	case aiPrimitiveType_POLYGON:
		pt = PrimitiveType::Polygon;
		primitiveIndexCount = 4;
		break;
	default:
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		break;
	}

	std::vector<Index_t> indices;
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

Mesh* MeshGroup::addMesh(const std::vector<Vertex_t>& vertices, const std::vector<Index_t>& indices, PrimitiveType pt) {
	auto mesh = std::make_unique<Mesh>();
	mesh->fill(vertices, indices, pt);
	m_meshes.push_back(std::move(mesh));
	return m_meshes.back().get();
}

Mesh* MeshGroup::addMesh(std::vector<Vertex_t>&& vertices, std::vector<Index_t>&& indices, PrimitiveType pt) {
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
