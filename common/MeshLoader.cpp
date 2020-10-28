#include"MeshLoader.h"
#include"Model.h"
#include"Skeleton.h"
#include"AnimationClip.h"
#include"Mesh.h"
#include"MaterialMgr.h"
#include"TextureMgr.h"
#include<assimp/Importer.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<queue>
#include<stack>
#include<unordered_set>


Model* MeshLoader::load(const std::string& file, int options, Preset preset,std::string name) {
	if (name.empty())
		name = ExtractFileNameFromPath(file);

	Assimp::Importer importer;
	auto aScene = importer.ReadFile(file, unsigned int (preset));
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
	model->setName(name);
	model->setFilePath(file);
	model->setSkeleton(skeleton);
	for (auto anim : animations) {
		model->addAnimation(anim);
	}

	loadMeshes(aScene, aScene->mRootNode, model, options, aiMatrix4x4());

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
	
	// load invert bind pose
	// first convert res pose joints transform to world space
	Pose worldBindPose = resPose.getGlobalPose();
	
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

		aiMatrix4x4 bindPoseMat = bone.second->mOffsetMatrix;
		bindPoseMat = bindPoseMat.Inverse().Transpose();
		worldBindPose[boneId] = mat2Transform(glm::make_mat4(&bindPoseMat[0][0]));
	}

	// invert world bind pose result a invBindPose
	Pose invBindPose;
	invBindPose.resize(worldBindPose.size());
	for (int i = 0; i < worldBindPose.size(); i++) {
		invBindPose[i] = inverse(worldBindPose[i]);
	}

	// load joint names
	std::vector<std::string> names;
	names.reserve(joints.size());
	for (auto joint : joints) {
		names.push_back(joint->mName.C_Str());
	}

	// invert root tansform
	aiMatrix4x4 rootTransform = aScene->mRootNode->mTransformation;
	glm::mat4 invRootTransform = glm::make_mat4(&rootTransform.Inverse().Transpose()[0][0]);
	
	Skeleton* skeleton = new Skeleton();
	skeleton->set(resPose, invBindPose, names, invRootTransform);

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
				posTrack.m_defualtVal = skeleton->getResPose().getJointTransformLocal(skeleton->getJointId(nodeAnim->mNodeName.C_Str())).position;

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
				scaleTrack.m_defualtVal = skeleton->getResPose().getJointTransformLocal(skeleton->getJointId(nodeAnim->mNodeName.C_Str())).scale;

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
				rotateTrack.m_defualtVal = skeleton->getResPose().getJointTransformLocal(skeleton->getJointId(nodeAnim->mNodeName.C_Str())).rotation;

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


void MeshLoader::loadMeshes(const aiScene* aScene, const aiNode* node, Model* model, int options, const aiMatrix4x4& parentTransform) {
	aiMatrix4x4 transform = parentTransform * node->mTransformation;
	for (size_t i = 0; i < node->mNumMeshes; i++) {
		const aiMesh* aMesh = aScene->mMeshes[node->mMeshes[i]];
		IMesh* mesh = model->hasSkeleton() ? loadGeometrys<SkinVertex_t>(aMesh, model->getSkeleton()) : loadGeometrys<Vertex_t>(aMesh, nullptr);
		if (!mesh)
			continue;

		mesh->setTransform(glm::transpose(glm::make_mat4(&transform[0][0])));
		model->addMesh(mesh);
		std::cout << mesh << std::endl;
		if (options & Option::LoadMaterials) {
			Material* mat = loadMaterial(aScene, aMesh);
			if (mat) {
				mat->setName(model->getName() + "_" + mat->getName());
				if (MaterialManager::getInstance()->addMaterial(mat))
					model->addEmbededMaterial(mesh, mat);
				else
					delete mat;
			}
		}
	}

	for (size_t j = 0; j < node->mNumChildren; j++) {
		loadMeshes(aScene, node->mChildren[j], model, options, transform);
	}
}


template<typename Vertex>
IMesh* MeshLoader::loadGeometrys(const aiMesh* aMesh, const Skeleton* skeleton) {
	std::vector<Vertex> vertices;
	std::vector<Index_t> indices;
	PrimitiveType pt = PrimitiveType::Unknown;
	vertices.reserve(aMesh->mNumVertices);

	for (size_t i = 0; i < aMesh->mNumVertices; i++) {
#ifdef _DEBUG
		ASSERT(aMesh->HasPositions());
		ASSERT(aMesh->HasNormals());
#endif // _DEBUG

		Vertex v;
		const aiVector3D pos = aMesh->mVertices[i];
		const aiVector3D normal = aMesh->mNormals[i];
		const aiVector3D tangent = aMesh->HasTangentsAndBitangents() ? aMesh->mTangents[i] : aiVector3D();
		const aiVector3D uv = aMesh->HasTextureCoords(0) ? aMesh->mTextureCoords[0][i] : aiVector3D();

		v.position = glm::vec3(pos.x, pos.y, pos.z);
		v.normal = glm::vec3(normal.x, normal.y, normal.z);
		v.tangent = glm::vec3(tangent.x, tangent.y, tangent.z);
		v.uv = glm::vec2(uv.x, uv.y);
		vertices.push_back(v);
	}

	if (vertices.empty())
		return nullptr;

	loadBoneWeights(vertices, aMesh, skeleton);

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
		return nullptr;
	}

	indices.clear();
	indices.reserve(aMesh->mNumFaces * primitiveIndexCount);

	for (size_t j = 0; j < aMesh->mNumFaces; j++) {
		const aiFace face = aMesh->mFaces[j];
		for (size_t k = 0; k < face.mNumIndices; k++) {
			indices.push_back(face.mIndices[k]);
		}
	}


	TMesh<Vertex>* mesh = new TMesh<Vertex>();
	mesh->fill(std::move(vertices), std::move(indices));
	mesh->setName(aMesh->mName.C_Str());
	mesh->setPrimitiveType(pt);

	return mesh;
}

template<>
void MeshLoader::loadBoneWeights<Vertex_t>(std::vector<Vertex_t>& vertices, const aiMesh* mesh, const Skeleton* skeleton) {

}


template<>
void MeshLoader::loadBoneWeights<SkinVertex_t>(std::vector<SkinVertex_t>& vertices, const aiMesh* aMesh, const Skeleton* skeleton) {
	if (aMesh->HasBones()) {
		for (size_t i = 0; i < aMesh->mNumBones; i++) {
			const aiBone* bone = aMesh->mBones[i];
			int boneId = skeleton->getJointId(bone->mName.C_Str());
			for (size_t j = 0; j < bone->mNumWeights; j++) {
				const aiVertexWeight& weight = bone->mWeights[j];
				setVertexWeight(vertices[weight.mVertexId], boneId, weight.mWeight);
			}
		}
	}
}


Material* MeshLoader::loadMaterial(const aiScene* aScene, const aiMesh* aMesh) {
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

	auto mat = new Material();
	mat->m_diffuseColor = glm::vec3(aiDiffuseColor.r, aiDiffuseColor.g, aiDiffuseColor.b);
	mat->m_specularColor = glm::vec3(aiSpecularColor.r, aiSpecularColor.g, aiSpecularColor.b);
	mat->m_emissiveColor = glm::vec3(aiEmissiveColor.r, aiEmissiveColor.g, aiEmissiveColor.b);
	mat->m_opacity = aiOpacity;
	mat->m_shininess = aiShininess;
	
	std::string name(aMesh->mName.C_Str()); 
	name = name + "_" + aiName.C_Str();
	mat->setName(name);

	if (hasDiffuseMap) {
		mat->m_diffuseMap = loadTexture(diffuseTextureName);
		if (mat->m_diffuseMap.expired()) {
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
		if (mat->m_normalMap.expired()) {
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
		if (mat->m_specularMap.expired()) {
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
		if (mat->m_emissiveMap.expired()) {
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


std::weak_ptr<Texture> MeshLoader::loadTexture(const std::string& name) {
	auto textureMgr = TextureManager::getInstance();
	if (textureMgr->hasTexture(name))
		return textureMgr->getTexture(name);

	return textureMgr->addTexture(name);
}


void MeshLoader::setVertexWeight(SkinVertex_t& vertex, int jointId, float weight) {
	int idx = 0;
	while (vertex.weights[idx] > 0) {
		idx++;
		if (idx >= 4) {
			std::cerr << "[Mesh Load Warning] Ignore Bone: " << jointId << ", weight: " << weight << std::endl;
			return;
		}
	}

	vertex.weights[idx] = weight;
	vertex.joints[idx] = jointId;
}



template IMesh* MeshLoader::loadGeometrys<Vertex_t>(const aiMesh* mesh, const Skeleton* skeleton);
template IMesh* MeshLoader::loadGeometrys<SkinVertex_t>(const aiMesh* mesh, const Skeleton* skeleton);
template void MeshLoader::loadBoneWeights<Vertex_t>(std::vector<Vertex_t>& vertices, const aiMesh* mesh, const Skeleton* skeleton);
template void MeshLoader::loadBoneWeights<SkinVertex_t>(std::vector<SkinVertex_t>& vertices, const aiMesh* mesh, const Skeleton* skeleton);
