#include"MeshGroup.h"
#include"Util.h"
#include"TextureMgr.h"
#include"MaterialMgr.h"
#include<assimp/scene.h>
#include<glm/gtc/type_ptr.hpp>



MeshGroup::MeshGroup() :m_name()
, m_id(0)
, m_file()
, m_meshes()
, m_embededMaterials() {
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

void MeshGroup::addMesh(const aiScene* scene, const aiMesh* mesh, aiMatrix4x4 transform, MeshLoadOption options) {
	std::vector<Vertex_t> vertices;
	std::vector<Index_t> indices;
	PrimitiveType pt = PrimitiveType::Unknown;
	
	// load vertices indics
	loadGeometry(mesh, vertices, indices, pt);
	
	if (vertices.empty())
		return;

	aiMatrix4x4 colMajorTransform = transform.Transpose();
	Mesh* newMesh = addMesh(std::move(vertices), std::move(indices), pt);
	newMesh->m_name = mesh->mName.C_Str();
	newMesh->m_transform = glm::make_mat4(&colMajorTransform[0][0]);

	// load material
	if (options & MeshLoadOption::LoadMaterial) {
		if (auto mat = loadMaterial(scene, mesh)) {
			MaterialManager::getInstance()->addMaterial(mat);
			m_embededMaterials.insert({ newMesh->id(), mat->getName() });
		}
	}
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


std::shared_ptr<Material> MeshGroup::embededMaterialForMesh(const Mesh* mesh) {
	return embededMaterialForMesh(mesh->id());
}

std::shared_ptr<Material> MeshGroup::embededMaterialForMesh(ID meshId) {
	auto pos = m_embededMaterials.find(meshId);
	if (pos == m_embededMaterials.end())
		return nullptr;

	return MaterialManager::getInstance()->getMaterial(pos->second);
}


void MeshGroup::loadGeometry(const aiMesh* aMesh, std::vector<Vertex_t>& vertices, std::vector<Index_t>& indices, PrimitiveType& pt) {
	vertices.clear();
	vertices.reserve(aMesh->mNumVertices);

	for (size_t i = 0; i < aMesh->mNumVertices; i++) {
		Vertex_t v;
		ASSERT(aMesh->HasPositions());
		const aiVector3D pos = aMesh->mVertices[i];

		ASSERT(aMesh->HasNormals());
		const aiVector3D normal = aMesh->mNormals[i];

		ASSERT(aMesh->HasTangentsAndBitangents());
		const aiVector3D tangent = aMesh->mTangents[i];
		//const aiVector3D biTangent = aMesh->mBitangents[i];

		const aiVector3D uv = aMesh->HasTextureCoords(0) ? aMesh->mTextureCoords[0][i] : aiVector3D();

		v.position = glm::vec3(pos.x, pos.y, pos.z);
		v.normal = glm::vec3(normal.x, normal.y, normal.z);
		v.tangent = glm::vec3(tangent.x, tangent.y, tangent.z);
		//v.biTangent = glm::vec3(biTangent.x, biTangent.y, biTangent.z);
		v.uv = glm::vec2(uv.x, uv.y);
		vertices.push_back(v);
	}

	if (vertices.empty())
		return;

	pt = PrimitiveType::Unknown;
	size_t primitiveIndexCount = 3;
	switch (aMesh->mPrimitiveTypes)
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

	indices.clear();
	indices.reserve(aMesh->mNumFaces * primitiveIndexCount);

	for (size_t j = 0; j < aMesh->mNumFaces; j++) {
		const aiFace face = aMesh->mFaces[j];
		for (size_t k = 0; k < face.mNumIndices; k++) {
			indices.push_back(face.mIndices[k]);
		}
	}
}

std::shared_ptr<Material> MeshGroup::loadMaterial(const aiScene* aScene, const aiMesh* aMesh) {
	if (!aScene->HasMaterials())
		return nullptr;

	if (aScene->mNumMaterials <= aMesh->mMaterialIndex)
		return nullptr;

	aiColor3D aiDiffuseColor(0.f, 0.f, 0.f);
	aiColor3D aiSpecularColor(1.f, 1.f, 1.f);
	aiColor3D aiEmissiveColor(0.f, 0.f, 0.f);
	aiString aiName;
	float aiOpacity = 1.f;
	float aiShininess = 0.f;

	bool hasDiffuseMap = false;
	bool hasNormalMap = false;
	bool hasEmissiveMap = false;
	bool hasSpecularMap = false;
	std::string diffuseTextureName;
	std::string normalTextureName;
	std::string specularTextureName;
	std::string emissiveTextureName;

	const aiMaterial* aMat = aScene->mMaterials[aMesh->mMaterialIndex];
	aMat->Get(AI_MATKEY_COLOR_DIFFUSE, aiDiffuseColor);
	aMat->Get(AI_MATKEY_COLOR_SPECULAR, aiSpecularColor);
	aMat->Get(AI_MATKEY_COLOR_EMISSIVE, aiEmissiveColor);
	aMat->Get(AI_MATKEY_OPACITY, aiOpacity);
	aMat->Get(AI_MATKEY_SHININESS, aiShininess);
	aMat->Get(AI_MATKEY_NAME, aiName);

	if (aMat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
		aiString path;
		aMat->GetTexture(aiTextureType_DIFFUSE, 0, &path);
		diffuseTextureName = ExtractFileNameFromPath(path.C_Str());
		hasDiffuseMap = !diffuseTextureName.empty();
	}

	if (aMat->GetTextureCount(aiTextureType_NORMALS) > 0) {
		aiString path;
		aMat->GetTexture(aiTextureType_NORMALS, 0, &path);
		normalTextureName = ExtractFileNameFromPath(path.C_Str());
		hasNormalMap = !normalTextureName.empty();
	}

	if (aMat->GetTextureCount(aiTextureType_SPECULAR) > 0) {
		aiString path;
		aMat->GetTexture(aiTextureType_SPECULAR, 0, &path);
		specularTextureName = ExtractFileNameFromPath(path.C_Str());
		hasSpecularMap = !specularTextureName.empty();
	}

	if (aMat->GetTextureCount(aiTextureType_EMISSIVE) > 0) {
		aiString path;
		aMat->GetTexture(aiTextureType_EMISSIVE, 0, &path);
		emissiveTextureName = ExtractFileNameFromPath(path.C_Str());
		hasEmissiveMap = !emissiveTextureName.empty();
	}

	auto mat = std::make_shared<Material>();
	mat->m_diffuseColor = glm::vec3(aiDiffuseColor.r, aiDiffuseColor.g, aiDiffuseColor.b);
	mat->m_specularColor = glm::vec3(aiSpecularColor.r, aiSpecularColor.g, aiSpecularColor.b);
	mat->m_emissiveColor = glm::vec3(aiEmissiveColor.r, aiEmissiveColor.g, aiEmissiveColor.b);
	mat->m_opacity = aiOpacity;
	mat->m_shininess = aiShininess;
	mat->setName(m_name + "_" + aMesh->mName.C_Str() + "_" + aiName.C_Str());
	if (hasDiffuseMap) {
		mat->m_diffuseMap = loadTexture(diffuseTextureName);
		if (!mat->m_diffuseMap) {
#ifdef _DEBUG	
			std::string msg;
			msg += "[Material Load error] Failed to load texture: ";
			msg += (mat->getName() + "_" + diffuseTextureName);
			CONSOLELOG(msg);
#endif // _DEBUG

		}
	}
	if (hasNormalMap) {
		mat->m_normalMap = loadTexture(normalTextureName);
		if (!mat->m_normalMap) {
#ifdef _DEBUG
			std::string msg;
			msg += "[Material Load error] Failed to load texture: ";
			msg += (mat->getName() + "_" + normalTextureName);
			CONSOLELOG(msg);
#endif // _DEBUG

		}
	}
	if (hasSpecularMap) {
		mat->m_specularMap = loadTexture(specularTextureName);
			if (!mat->m_specularMap) {
#ifdef _DEBUG
				std::string msg;
				msg += "[Material Load error] Failed to load texture: ";
				msg += (mat->getName() + "_" + specularTextureName);
				CONSOLELOG(msg);
#endif // _DEBUG

			}
	}
	if (hasEmissiveMap) {
		mat->m_emissiveMap = loadTexture(emissiveTextureName);
			if (!mat->m_emissiveMap) {
#ifdef _DEBUG
				std::string msg;
				msg += "[Material Load error] Failed to load texture: ";
				msg += (mat->getName() + "_" + emissiveTextureName);
				CONSOLELOG(msg);
#endif // _DEBUG

			}
	}

	return mat;
}


std::shared_ptr<Texture> MeshGroup::loadTexture(const std::string& name) {
	auto textureMgr = TextureManager::getInstance();
	auto loadedTex = textureMgr->getTexture(name);
	
	if (loadedTex != nullptr)
		return loadedTex;

	return textureMgr->addTexture(textureMgr->getResourceAbsolutePath() + name, name);
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
