#pragma once
#include"Singleton.h"
#include"Model.h"
#include"Skeleton.h"
#include"AnimationClip.h"
#include<string>
#include<assimp/postprocess.h>
#include<assimp/scene.h>
#include<unordered_map>
#include<vector>


class MeshLoader : public Singleton<MeshLoader> {
public:
	enum Option {
		None = 0,
		LoadMaterials = 1 << 0,
		LoadAnimations = 1 << 1,
	};

	enum Preset {
		Fast = aiProcessPreset_TargetRealtime_Fast,
		Quality = aiProcessPreset_TargetRealtime_Quality,
		MaxQuality = aiProcessPreset_TargetRealtime_MaxQuality,
	};

	Model* load(const std::string& file, Option options = Option::None, Preset preset = Preset::Quality ,const std::string& name = "");

protected:
	Skeleton* loadSkeletion(const aiScene* aScene);
	std::vector<AnimationClip*> loadAnimations(const aiScene* aScene, const Skeleton* skeleton);
	const aiNode* findSkeletonRootNode(const aiNode* node, const std::unordered_map<std::string, aiBone*>& bones);

};