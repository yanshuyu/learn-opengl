#include"MeshRenderComponent.h"
#include"SceneObject.h"
#include"MaterialMgr.h"
#include"RendererCore.h"
#include"Renderer.h"


RTTI_IMPLEMENTATION(MeshRenderComponent)

MeshRenderComponent::MeshRenderComponent(std::weak_ptr<Model> meshes, bool useEmbededMaterial):m_meshes(meshes)
, m_materials() {
	setMeshes(meshes, useEmbededMaterial);
}


void MeshRenderComponent::setMeshes(std::weak_ptr<Model> meshes, bool useEmbededMaterials) {
	m_meshes = meshes;
	m_materials.clear();

	if (!m_meshes.expired() && useEmbededMaterials)
		loadEmbededMaterials();
	
}


Component* MeshRenderComponent::copy() const {
	MeshRenderComponent* copyed = new MeshRenderComponent();
	copyed->m_meshes = m_meshes;
	copyed->m_materials = m_materials;
	copyed->m_isEnable = m_isEnable;
	
	return copyed;
}


MeshRenderComponent* MeshRenderComponent::create() {
	return new MeshRenderComponent();
}

void MeshRenderComponent::destory(MeshRenderComponent* mrc){
	if (mrc)
		delete mrc;
}


void MeshRenderComponent::addMaterial(std::weak_ptr<IMaterial> mtl) {
	m_materials.push_back(mtl);
}

void MeshRenderComponent::setMaterialAt(size_t index, std::weak_ptr<IMaterial> mtl) {
	m_materials[index] = mtl;
}

std::weak_ptr<IMaterial> MeshRenderComponent::materialAt(size_t index) const {
	if (index < 0 || index >= m_materials.size())
		return std::weak_ptr<IMaterial>();
	
	return m_materials[index];
}

void MeshRenderComponent::swapMaterial(size_t formIndex, size_t toIndex) {
	std::swap(m_materials[formIndex], m_materials[toIndex]);
}

void MeshRenderComponent::moveMaterial(size_t srcIndex, size_t dstIndex) {
	if (srcIndex > dstIndex) {
		auto moved = m_materials[srcIndex];
		size_t index = srcIndex - 1;
		while (index >= dstIndex)
			m_materials[index + 1] = m_materials[index];
		m_materials[dstIndex] = moved;

	} else if (srcIndex < dstIndex) {
		auto moved = m_materials[srcIndex];
		size_t index = srcIndex + 1;
		while (index <= dstIndex)
			m_materials[index - 1] = m_materials[index];
		m_materials[dstIndex] = moved;
	}
}

void MeshRenderComponent::render(RenderContext* context) {
	if (!m_isEnable || m_meshes.expired())
		return;

	auto model = m_meshes.lock();

	if (model->meshCount() <= 0)
		return;

	for (size_t i = 0; i < model->meshCount(); i++) {
		MeshRenderItem_t task;
		std::shared_ptr<IMaterial> mtl = materialAt(i).expired() ? nullptr : materialAt(i).lock();
		auto mesh = model->meshAt(i);
		auto mat = mtl ? mtl.get() : MaterialManager::getInstance()->defaultPhongMaterial();

		task.vao = mesh->vertexArray();
		task.vertexCount = mesh->verticesCount();
		task.indexCount = mesh->indicesCount();
		task.primitive = mesh->getPrimitiveType();
		task.material = mat;
		task.modelMatrix = context->getMatrix() * m_owner->m_transform.getMatrix() * mesh->getTransform();
		auto layer = getGameObject()->getLayer();
		auto renderer = context->getRenderer();
		if (layer == SceneLayer::Opaque) {
			renderer->submitOpaqueItem(task);
		}
		else if (layer == SceneLayer::CutOut) {
			renderer->submitCutOutItem(task);
		}
		else if (layer == SceneLayer::Transparency) {
			renderer->submitTransparentItem(task);
		}
	}
}


void MeshRenderComponent::loadEmbededMaterials() {
	if (m_meshes.expired())
		return;

	auto model = m_meshes.lock();

	if (model->embededMaterialCount() > 0) {
		m_materials.resize(model->meshCount());
		for (size_t i = 0; i < model->meshCount(); i++) {
			m_materials[i] = model->embededMaterialForMesh(model->meshAt(i));
		}
	}
}


