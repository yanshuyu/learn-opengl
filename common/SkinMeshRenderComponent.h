#pragma once
#include"MeshRenderComponent.h"
#include"AnimatorComponent.h"

class Pose;


class SkinMeshRenderComponent : public MeshRenderComponent {

	RTTI_DECLARATION(SkinMeshRenderComponent)

public:
	SkinMeshRenderComponent(std::weak_ptr<Model> meshes = std::weak_ptr<Model>(),
		std::weak_ptr<AnimatorComponent> animator = std::weak_ptr<AnimatorComponent>(),  
		bool useEmbededMaterial = true);

	void setMeshes(std::weak_ptr<Model> meshes, bool useEmbededMaterials = true) override;

	void setAnimator(std::weak_ptr<AnimatorComponent> animator);

	void render(RenderContext* context) override;

	inline std::weak_ptr<AnimatorComponent> getAnimator() const {
		return m_animator;
	}

protected:
	std::vector<glm::mat4> m_invBindPoseTransform;
	std::vector<glm::mat4> m_bonesTransform;
	std::weak_ptr<AnimatorComponent> m_animator;
	glm::mat4 m_invRootTransform;
};