#include"MeshRenderComponent.h"
#include"SceneObject.h"
#include"MaterialMgr.h"
#include"Renderer.h"


const std::string MeshRenderComponent::s_identifier = "MeshRender";

MeshRenderComponent::MeshRenderComponent(std::shared_ptr<MeshGroup> meshes, bool useEmbededMaterial):m_meshes(meshes)
, m_materials() {
	if (meshes && useEmbededMaterial)
		loadEmbededMaterials();
}


void MeshRenderComponent::setMeshes(std::shared_ptr<MeshGroup> meshes, bool useEmbededMaterials) {
	m_meshes = meshes;
	m_materials.clear();

	if (m_meshes && useEmbededMaterials)
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


size_t MeshRenderComponent::meshCount() const {
	if (m_meshes)
		return m_meshes->meshesCount();
	return 0;
}

void MeshRenderComponent::addMaterial(std::shared_ptr<Material> m) {
	m_materials.push_back(m);
}

void MeshRenderComponent::setMaterialAt(size_t index, std::shared_ptr<Material> m) {
	m_materials[index] = m;
}

std::shared_ptr<Material> MeshRenderComponent::materialAt(size_t index) const {
	if (index >= m_materials.size())
		return nullptr;
	
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
	if (!m_isEnable || meshCount() <= 0)
		return;

	for (size_t i = 0; i < meshCount(); i++) {
		RenderTask_t task;
		auto mesh = m_meshes->meshAt(i);
		auto material = materialAt(i) ? materialAt(i).get() : MaterialManager::defaultMaterial();

		task.vao = mesh->vertexArray();
		task.vertexCount = mesh->verticesCount();
		task.indexCount = mesh->indicesCount();
		task.primitive = mesh->getPrimitiveType();
		task.material = material;
		task.modelMatrix = context->getMatrix() * m_owner->m_transform.getMatrix() * mesh->getTransform(); 
		context->renderer()->subsimtTask(task);
	}
}


void MeshRenderComponent::loadEmbededMaterials() {
	if (m_meshes && m_meshes->embededMaterialCount() > 0) {
		m_materials.resize(m_meshes->meshesCount(), nullptr);
		for (size_t i = 0; i < m_meshes->meshesCount(); i++) {
			m_materials[i] = m_meshes->embededMaterialForMesh(m_meshes->meshAt(i));
		}
	}
}


