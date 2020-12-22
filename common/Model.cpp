#include"Model.h"
#include"Util.h"
#include"Skeleton.h"
#include"AnimationClip.h"
#include"TextureMgr.h"
#include"MaterialMgr.h"
#include<assimp/scene.h>
#include<glm/gtc/type_ptr.hpp>


Model::Model() :m_name()
, m_id(0)
, m_file()
, m_meshes()
, m_embededMaterials()
, m_skeleton()
, m_animations() {
	m_id = reinterpret_cast<ID>(this);
}

Model::Model(Model&& rv) noexcept :Model() {
	*this = std::move(rv);
}

Model& Model::operator = (Model&& rv) noexcept {
	if (&rv == this)
		return *this;

	m_name = std::move(rv.m_name);
	m_file = std::move(rv.m_file);
	m_meshes = std::move(rv.m_meshes);
	m_embededMaterials = std::move(rv.m_embededMaterials);
	m_skeleton = std::move(rv.m_skeleton);
	m_animations = std::move(rv.m_animations);

	return *this;
}

Model::~Model() {
	release();
}

void Model::release() {
	m_file.clear();
	m_name.clear();

	m_meshes.clear();
	m_embededMaterials.clear();

	m_animations.clear();
	m_skeleton.release();
}


void Model::addMesh(IMesh* mesh) {
	m_meshes.emplace_back(mesh);
}

void Model::addMesh(std::unique_ptr<IMesh>&& mesh) {
	m_meshes.push_back(std::move(mesh));
}

void Model::setSkeleton(Skeleton* skeleton) {
	m_skeleton.reset(skeleton);
}

void Model::addAnimation(AnimationClip* anim) {
	m_animations.emplace_back(anim);
}

std::vector<IMesh*> Model::getMeshes() const {
	std::vector<IMesh*> meshes;
	meshes.reserve(m_meshes.size());
	for (auto& mesh : m_meshes) {
		meshes.push_back(mesh.get());
	}

	return meshes;
}

std::vector<AnimationClip*> Model::getAnimations() const {
	std::vector<AnimationClip*> animations;
	animations.reserve(m_animations.size());
	for (auto& anim : m_animations) {
		animations.push_back(anim.get());
	}

	return animations;
}

bool Model::addEmbededMaterial(const IMesh* mesh, const IMaterial* mat) {
	return m_embededMaterials.insert({ mesh->id(), mat->getName() }).second;
}

std::weak_ptr<IMaterial> Model::embededMaterialForMesh(const IMesh* mesh) {
	return embededMaterialForMesh(mesh->id());
}

std::weak_ptr<IMaterial> Model::embededMaterialForMesh(ID meshId) {
	auto pos = m_embededMaterials.find(meshId);
	if (pos == m_embededMaterials.end())
		return std::weak_ptr<IMaterial>();

	return MaterialManager::getInstance()->getMaterial(pos->second);
}



std::ostream& operator << (std::ostream& o, const Model& model) {
	o << "Model name: " << model.getName() << "(" << model.id() << "),\n";
	o << "filePath: " << model.getFilePath() << ",\n";
	o << "meshes(cout:" << model.meshCount() << ")";
	for (size_t i = 0; i < model.meshCount(); i++) {
		o << " " << model.meshAt(i) << "\n";
	}
	o << "\n";
	return o;
}



