#pragma once
#include"Mesh.h"
#include"Material.h"
#include"Skeleton.h"
#include"AnimationClip.h"
#include<assimp/Importer.hpp>
#include<assimp/mesh.h>
#include<unordered_map>




class Model {
	friend class MeshLoader;
	friend class MeshManager;

public:
	Model();
	Model(Model&& other) noexcept;
	Model& operator = (Model&& rv) noexcept;
	~Model();

	Model(const Model& other) = delete;
	Model& operator = (const Model& other) = delete;

	void release();

	void addMesh(const aiScene* scene, const aiMesh* mesh, aiMatrix4x4 transform, MeshLoadOption options);
	void addMesh(Mesh* mesh);
	void addMesh(std::unique_ptr<Mesh>&& mesh);
	Mesh* addMesh(const std::vector<Vertex_t>& vertices, const std::vector<Index_t>& indices, PrimitiveType pt = PrimitiveType::Triangle);
	Mesh* addMesh(std::vector<Vertex_t>&& vertices, std::vector<Index_t>&& indices, PrimitiveType pt = PrimitiveType::Triangle);

	void setSkeleton(Skeleton* skeleton);
	void addAnimation(AnimationClip* anim);

	std::vector<const Mesh*> getMeshes() const;
	std::vector<AnimationClip*> getAnimations() const;

	std::shared_ptr<Material> embededMaterialForMesh(const Mesh* mesh);
	std::shared_ptr<Material> embededMaterialForMesh(ID meshId);

	inline ID id() const {
		return m_id;
	}

	inline std::string filePath() const {
		return m_file;
	}

	inline std::string getName() const {
		return m_name;
	}

	inline void setName(const std::string& name) {
		m_name = name;
	}

	inline size_t meshCount() const {
		return m_meshes.size();
	}

	inline const Mesh* meshAt(size_t idx) const {
		return m_meshes.at(idx).get();
	}

	inline size_t animationCount() const {
		return m_animations.size();
	}

	inline AnimationClip* animationAt(size_t idx) const {
		return m_animations[idx].get();
	}

	inline bool hasSkeleton() const {
		return m_skeleton != nullptr;
	}

	inline const Skeleton* getSkeleton() const {
		return m_skeleton.get();
	}

	inline size_t embededMaterialCount() const {
		return m_embededMaterials.size();
	}

private:
	void loadGeometry(const aiMesh* aMesh, std::vector<Vertex_t>& vertices, std::vector<Index_t>& indices, PrimitiveType& pt);
	std::shared_ptr<Material> loadMaterial(const aiScene* aScene, const aiMesh* aMesh);
	std::shared_ptr<Texture> loadTexture(const std::string& name);

private:
	std::string m_file;
	std::string m_name;
	ID m_id;

	std::vector<std::unique_ptr<Mesh>> m_meshes;
	std::unordered_map<ID, std::string> m_embededMaterials;

	std::unique_ptr<Skeleton> m_skeleton;
	std::vector<std::unique_ptr<AnimationClip>> m_animations;
};


std::ostream& operator << (std::ostream& o, const Model& meshGrp);