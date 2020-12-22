#include"SkinMeshRenderComponent.h"
#include"Skeleton.h"
#include"MaterialMgr.h"
#include"SceneObject.h"
#include"Renderer.h"


RTTI_IMPLEMENTATION(SkinMeshRenderComponent)


SkinMeshRenderComponent::SkinMeshRenderComponent(std::weak_ptr<Model> meshes, std::weak_ptr<AnimatorComponent> animator, bool useEmbededMaterial)
: m_animator(animator)
, m_invBindPoseTransform()
, m_bonesTransform()
, m_invRootTransform(1.f) {
	setMeshes(meshes, useEmbededMaterial);
}


void SkinMeshRenderComponent::render(RenderContext* context) {
	if (!m_isEnable || !isMeshValid())
		return;

	auto model = m_meshes.lock();
	
	if (model->meshCount() <= 0)
		return;

	if (m_animator.expired())
		return __super::render(context);

	const auto& interpolatedPose = m_animator.lock()->animatedPose();
	interpolatedPose.getSkinMatrix(m_bonesTransform);
	
	for (size_t i = 0; i < m_bonesTransform.size(); i++) {
		m_bonesTransform[i] =  m_invRootTransform * m_bonesTransform[i] * m_invBindPoseTransform[i];
	}

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
		task.modelMatrix = context->getMatrix() * m_owner->m_transform.getMatrix(); // mesh->getTransform();
		task.bonesTransform = m_bonesTransform.data();
		task.boneCount = m_bonesTransform.size();

		auto layer = getGameObject()->getLayer();
		if (layer == SceneLayer::Default) {
			context->getRenderer()->submitOpaqueItem(task);
		} else if(layer == SceneLayer::CutOut) {
			context->getRenderer()->submitCutOutItem(task);
		}
	}
}

void SkinMeshRenderComponent::setMeshes(std::weak_ptr<Model> meshes, bool useEmbededMaterials) {
	__super::setMeshes(meshes, useEmbededMaterials);
	if (!m_meshes.expired()) {
		auto model = m_meshes.lock();
		if (model->hasSkeleton()) {
			model->getSkeleton()->getInvBindPose().getSkinMatrix(m_invBindPoseTransform);
			m_invRootTransform = model->getSkeleton()->getInvRootTransform();
		}
		
		//if (!m_animator.expired())
		//	m_animator.lock()->setAvater(m_meshes);
	}
}


void SkinMeshRenderComponent::setAnimator(std::weak_ptr<AnimatorComponent> animator) {
	m_animator = animator;
	//if (!m_animator.expired())
	//	m_animator.lock()->setAvater(m_meshes);
}