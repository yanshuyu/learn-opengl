#pragma once
#include"Singleton.h"
#include"IMesh.h"
#include<string>
#include<assimp/postprocess.h>
#include<assimp/scene.h>
#include<unordered_map>
#include<vector>

class Model;
class AnimationClip;
class Skeleton;
class Material;
class Texture;


class MeshLoader : public Singleton<MeshLoader> {
public:
	enum Option {
		None = 0,
		LoadMaterials = 1 << 0,
		LoadAnimations = 1 << 1,
	};

	enum class Preset {
		Fast = aiProcessPreset_TargetRealtime_Fast,
		Quality = aiProcessPreset_TargetRealtime_Quality,
		MaxQuality = aiProcessPreset_TargetRealtime_MaxQuality,
	};

	Model* load(const std::string& file, int options = Option::None, Preset preset = Preset::Quality ,const std::string& name = "");

protected:
	Skeleton* loadSkeletion(const aiScene* aScene);
	std::vector<AnimationClip*> loadAnimations(const aiScene* aScene, const Skeleton* skeleton);
	const aiNode* findSkeletonRootNode(const aiNode* node, const std::unordered_map<std::string, aiBone*>& bones);

	void loadMeshes(const aiScene* aScene, const aiNode* node, Model* model, int options, const aiMatrix4x4& parentTransform);
	Material* loadMaterial(const aiScene* aScene, const aiMesh* mesh);
	std::weak_ptr<Texture> loadTexture(const std::string& name);
	
	template<typename Vertex>
	IMesh* loadGeometrys(const aiMesh* mesh, const Skeleton* skeleton);
	
	template<typename Vertex>
	void loadBoneWeights(std::vector<Vertex>& vertices, const aiMesh* mesh, const Skeleton* skeleton);

	void setVertexWeight(SkinVertex_t& vertex, int jointId, float weight);
};