#include"MeshLoader.h"
#include<assimp/Importer.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<queue>
#include<stack>
#include<unordered_set>



Model* MeshLoader::load(const std::string& file, Option options, Preset preset, const std::string& name) {
	std::string modelName(name);
	if (modelName.empty())
		modelName = ExtractFileNameFromPath(file);
	if (modelName.empty())
		modelName = file;

	Assimp::Importer importer;
	auto aScene = importer.ReadFile(file, preset);
	if (!aScene) {
#ifdef _DEBUG
		std::cerr << "[Mesh Load error] Failed to load model: \"" << file << "\", reason: " << importer.GetErrorString() << std::endl;
#endif // _DEBUG
		return nullptr;
	}

	Skeleton* skeleton = nullptr;
	std::vector<AnimationClip*> animations;
	if (options & Option::LoadAnimations && aScene->mNumAnimations > 0) {
		skeleton = loadSkeletion(aScene);
		animations = loadAnimations(aScene, skeleton);
	}

	auto model = new Model();
	model->setSkeleton(skeleton);
	for (auto anim : animations) {
		model->addAnimation(anim);
	}

	return model;
}


Skeleton* MeshLoader::loadSkeletion(const aiScene* aScene) {
	std::unordered_map<std::string, aiBone*> bones;
	
	for (size_t i = 0; i < aScene->mNumMeshes; i++) { // for every mesh
		aiMesh* mesh = aScene->mMeshes[i];
		for (size_t j = 0; j < mesh->mNumBones; ++j) { // for every influent bone of mesh
			aiBone* bone = mesh->mBones[j];
			bones.insert({ bone->mName.C_Str(), bone });
		}
	}

	if (bones.size() <= 0)
		return nullptr;

	auto rootBone = findSkeletonRootNode(aScene->mRootNode, bones);
	if (!rootBone)
		return nullptr;
	
	std::stack<const aiNode*> rootBoneParents; 
	aiNode* rootParent = rootBone->mParent;
	while (rootParent) {
		rootBoneParents.push(rootParent);
		rootParent = rootParent->mParent;
	}

	// flatten skeleton to array
	std::vector<const aiNode*> joints;
	std::vector<int> parents;
	std::unordered_map<std::string, int> jointIdMap;
	std::queue<const aiNode*> bftQueue;
	std::queue<int> parentIdQueue;

	while (!rootBoneParents.empty()) {
		joints.push_back(rootBoneParents.top());
		parents.push_back(parents.size() - 1);
		jointIdMap.insert({ rootBoneParents.top()->mName.C_Str(), joints.size() - 1 });
		rootBoneParents.pop();
	}

	bftQueue.push(rootBone);
	parentIdQueue.push(parents.size() - 1);
	while (!bftQueue.empty()) {
		auto joint = bftQueue.front();
		joints.push_back(joint);
		parents.push_back(parentIdQueue.front());
		
		int thisJointId = joints.size() - 1;
		jointIdMap.insert({ joint->mName.C_Str(), thisJointId});

		for (size_t i = 0; i < joint->mNumChildren; ++i) {
			bftQueue.push(joint->mChildren[i]);
			parentIdQueue.push(thisJointId);
		}

		bftQueue.pop();
		parentIdQueue.pop();
	}

	// load res pose
	Pose resPose;
	resPose.resize(joints.size());
	for (size_t i = 0; i < joints.size(); i++) {
		aiMatrix4x4 transform = joints[i]->mTransformation;
		transform = transform.Transpose();
		resPose[i] = mat2Transform(glm::make_mat4(&transform[0][0]));
		resPose.setJointParent(i, parents[i]);
	}

	// load bind pose
	// first convert res pose joints transform to world space
	Pose bindPose = resPose;
	
	/*
	// second using joints bind pose transform to overwrite res pose joints world transform
	// result a bind pose in world space
	for (auto bone : bones) {
		int boneId = -1;
		auto pos = jointIdMap.find(bone.first);
		if (pos != jointIdMap.end())
			boneId = pos->second;

		if (boneId < 0) {
#ifdef _DEBUG
			ASSERT(false);
#endif // _DEBUG
			continue;
		}

		aiMatrix4x4 offsetMat = bone.second->mOffsetMatrix; 
		offsetMat = offsetMat.Transpose();
		glm::mat4 invBindMat = glm::make_mat4(&offsetMat[0][0]);
		bindPose[boneId] = mat2Transform(glm::inverse(invBindMat));
	}

	// finally convert bind pose joints transform from world sapce to local space
	
	for (int i = bindPose.size() - 1; i >= 0; i--) {
		int parentId = bindPose.getJointParent(i);
		if (parentId >= 0) {
			bindPose[i] = combine(inverse(bindPose[parentId]), bindPose[i]);
		}
	}
	*/

	// load joint names
	std::vector<std::string> names;
	names.reserve(joints.size());
	for (auto joint : joints) {
		names.push_back(joint->mName.C_Str());
	}

	Skeleton* skeleton = new Skeleton();
	skeleton->set(resPose, bindPose, names);

	return skeleton;
}


std::vector<AnimationClip*> MeshLoader::loadAnimations(const aiScene* aScene, const Skeleton* skeleton) {
	std::vector<AnimationClip*> animations;
	animations.reserve(aScene->mNumAnimations);
	for (size_t i = 0; i < aScene->mNumAnimations; i++) { // fro every animation clip
		aiAnimation* anim = aScene->mAnimations[i];
		float dur = anim->mTicksPerSecond > 0 ? anim->mDuration / anim->mTicksPerSecond : anim->mDuration;
		
		if (dur < 0.1) // ignore animation that is to short
			continue;
		
		AnimationClip* animClip = new AnimationClip();
		animations.push_back(animClip);
		animClip->setName(anim->mName.C_Str());
		animClip->setDuration(dur);
		animClip->resize(anim->mNumChannels);
		
		for (size_t j = 0; j < anim->mNumChannels; j++) { // for every joint's animation track in clip
			aiNodeAnim* nodeAnim = anim->mChannels[j];
			TransformTrack& jointAnimTrack = animClip->trackAt(j);
			
			jointAnimTrack.setJointId(skeleton->getJointId(nodeAnim->mNodeName.C_Str()));
			
			if (nodeAnim->mNumPositionKeys > 0) {
				VectorTrack& posTrack = jointAnimTrack.getPositionTrack();
				posTrack.setInterpolationType(InterpolationType::Linear); 
				posTrack.resize(nodeAnim->mNumPositionKeys);
				posTrack.m_preBehavior = VectorTrack::Behavior(nodeAnim->mPreState);
				posTrack.m_postBehavior = VectorTrack::Behavior(nodeAnim->mPostState);
				posTrack.m_defualtVal = skeleton->getBindPose().getJointTransformLocal(skeleton->getJointId(nodeAnim->mNodeName.C_Str())).position;

				for (size_t p = 0; p < nodeAnim->mNumPositionKeys; p++) { // for every position key frame
					float t = anim->mTicksPerSecond > 0 ? nodeAnim->mPositionKeys[p].mTime / anim->mTicksPerSecond : nodeAnim->mPositionKeys[p].mTime;
					aiVector3D val = nodeAnim->mPositionKeys[p].mValue;
					posTrack[p].m_time = t;
					posTrack[p].m_value = glm::vec3(val.x, val.y, val.z);
				}
			}

			if (nodeAnim->mNumScalingKeys > 0) {
				VectorTrack& scaleTrack = jointAnimTrack.getScaleTrack();
				scaleTrack.setInterpolationType(InterpolationType::Linear);
				scaleTrack.resize(nodeAnim->mNumScalingKeys);
				scaleTrack.m_preBehavior = VectorTrack::Behavior(nodeAnim->mPreState);
				scaleTrack.m_postBehavior = VectorTrack::Behavior(nodeAnim->mPostState);
				scaleTrack.m_defualtVal = skeleton->getBindPose().getJointTransformLocal(skeleton->getJointId(nodeAnim->mNodeName.C_Str())).scale;

				for (size_t s = 0; s < nodeAnim->mNumScalingKeys; s++) { // for every scale key frame
					float t = anim->mTicksPerSecond > 0 ? nodeAnim->mScalingKeys[s].mTime / anim->mTicksPerSecond : nodeAnim->mScalingKeys[s].mTime;
					aiVector3D val = nodeAnim->mScalingKeys[s].mValue;
					scaleTrack[s].m_time = t;
					scaleTrack[s].m_value = glm::vec3(val.x, val.y, val.z);
				}
			}

			if (nodeAnim->mNumRotationKeys > 0) {
				QuaternionTrack& rotateTrack = jointAnimTrack.getRotationTrack();
				rotateTrack.setInterpolationType(InterpolationType::Linear);
				rotateTrack.resize(nodeAnim->mNumRotationKeys);
				rotateTrack.m_preBehavior = QuaternionTrack::Behavior(nodeAnim->mPreState);
				rotateTrack.m_postBehavior = QuaternionTrack::Behavior(nodeAnim->mPostState);
				rotateTrack.m_defualtVal = skeleton->getBindPose().getJointTransformLocal(skeleton->getJointId(nodeAnim->mNodeName.C_Str())).rotation;

				for (size_t r = 0; r < nodeAnim->mNumRotationKeys; r++) { // for every rotation key frame
					float t = anim->mTicksPerSecond > 0 ? nodeAnim->mRotationKeys[r].mTime / anim->mTicksPerSecond : nodeAnim->mRotationKeys[r].mTime;
					aiQuaternion val = nodeAnim->mRotationKeys[r].mValue;
					rotateTrack[r].m_time = t;
					rotateTrack[r].m_value = glm::quat(val.w, val.x, val.y, val.z);
				}
			}
		}
	}
   	return animations;
}


const aiNode* MeshLoader::findSkeletonRootNode(const aiNode* node, const std::unordered_map<std::string, aiBone*>& bones) {
	if (!node)
		return nullptr;

	std::queue<const aiNode*> rows;
	std::queue<size_t> nodesCountInRow;
	rows.push(node);
	nodesCountInRow.push(1);

	while (!nodesCountInRow.empty()) {
		size_t curRownodeCount = nodesCountInRow.front();
		std::vector<const aiNode*> bonesNodeInRow;
		size_t newRowNodeCount = 0;

		while (curRownodeCount--) {
			auto curNode = rows.front();
			if (bones.find(curNode->mName.C_Str()) != bones.end()) // a bone node in row
				bonesNodeInRow.push_back(curNode);

			rows.pop();
			for (size_t i = 0; i < curNode->mNumChildren; i++) {
				rows.push(curNode->mChildren[i]);
				newRowNodeCount++;
			}
		}

		if (bonesNodeInRow.size() == 1) // find only one bone in row
			return bonesNodeInRow.front();

		if (bonesNodeInRow.size() > 1) { // find multiple bones in row
			std::unordered_set<const aiNode*> parents;
			std::for_each(bonesNodeInRow.begin(), bonesNodeInRow.end(), [&](const aiNode* boneNode) {
				if (boneNode->mParent)
					parents.insert(boneNode->mParent);
				});

			while (parents.size() > 1) {
				std::unordered_set<const aiNode*> copy = parents;
				parents.clear();

				std::for_each(copy.begin(), copy.end(), [&](const aiNode* boneNode) {
					if (boneNode->mParent)
						parents.insert(boneNode->mParent);
					});
			}

			ASSERT(parents.size() == 1);
			return *parents.begin();
		}

		nodesCountInRow.pop();
		if (newRowNodeCount > 0)
			nodesCountInRow.push(newRowNodeCount);
	}

	return nullptr;
}