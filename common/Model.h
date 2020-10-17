#pragma once
#include"Mesh.h"
#include<assimp/Importer.hpp>
#include<assimp/mesh.h>
#include<unordered_map>


class Skeleton;
class AnimationClip;


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

	void addMesh(IMesh* mesh);
	void addMesh(std::unique_ptr<IMesh>&& mesh);

	template<typename Vertex>
	TMesh<Vertex>* addMesh(const std::vector<Vertex>& vertices, const std::vector<Index_t>& indices, PrimitiveType pt = PrimitiveType::Triangle) {
		std::vector<Vertex> tempVertices = vertices;
		std::vector<Index_t> tempIndices = indices;
		return addMesh(std::move(tempVertices), std::move(tempIndices), pt);
	}
	
	template<typename Vertex>
	TMesh<Vertex>* addMesh(std::vector<Vertex>&& vertices, std::vector<Index_t>&& indices, PrimitiveType pt = PrimitiveType::Triangle) {
		TMesh<Vertex>* mesh = new TMesh<Vertex>();
		mesh->fill(std::move(vertices), std::move(indices));
		mesh->setPrimitiveType(pt);
		addMesh(mesh);

		return mesh;
	}

	void setSkeleton(Skeleton* skeleton);
	void addAnimation(AnimationClip* anim);

	std::vector<IMesh*> getMeshes() const;
	std::vector<AnimationClip*> getAnimations() const;

	bool addEmbededMaterial(const IMesh* mesh, const Material* mat);
	std::shared_ptr<Material> embededMaterialForMesh(const IMesh* mesh);
	std::shared_ptr<Material> embededMaterialForMesh(ID meshId);

	inline ID id() const {
		return m_id;
	}

	inline void setFilePath(const std::string& path) {
		m_file = path;
	}

	inline std::string getFilePath() const {
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

	inline  IMesh* meshAt(size_t idx) const {
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
	std::string m_file;
	std::string m_name;
	ID m_id;

	std::vector<std::unique_ptr<IMesh>> m_meshes;	// geometry data
	std::unordered_map<ID, std::string> m_embededMaterials;

	std::unique_ptr<Skeleton> m_skeleton;	// skeleton animation data
	std::vector<std::unique_ptr<AnimationClip>> m_animations;
};


std::ostream& operator << (std::ostream& o, const Model& model);