#pragma once
#include"RenderableComponent.h"
#include"IMaterial.h"
#include"Model.h"
#include"RendererCore.h"

class MeshRenderComponent : public RenderableComponent {

	RTTI_DECLARATION(MeshRenderComponent)

public:
	MeshRenderComponent(std::weak_ptr<Model> meshes = std::weak_ptr<Model>(), bool useEmbededMaterial = true);
	MeshRenderComponent(MeshRenderComponent&& rv) = default;
	MeshRenderComponent& operator = (MeshRenderComponent&& rv) = default;

	MeshRenderComponent& operator = (const MeshRenderComponent& other) = delete;
	MeshRenderComponent(const MeshRenderComponent& other) = delete;

	Component* copy() const override;
	static MeshRenderComponent* create();
	static void destory(MeshRenderComponent* mrc);

	//
	// mesh managment
	//
	virtual void setMeshes(std::weak_ptr<Model> meshes, bool useEmbededMaterials = true);

	inline std::weak_ptr<Model> getMeshes() const {
		return m_meshes;
	}

	inline bool isMeshValid() const {
		return !m_meshes.expired();
	}

	
	//
	//material managment
	//
	void addMaterial(std::weak_ptr<IMaterial> mtl);
	void setMaterialAt(size_t index, std::weak_ptr<IMaterial> mtl);
	std::weak_ptr<IMaterial> materialAt(size_t index) const;
	void swapMaterial(size_t formIndex, size_t toIndex);
	void moveMaterial(size_t srcIndex, size_t dstIndex);

	inline const std::vector<std::weak_ptr<IMaterial>>& materials() const {
		return m_materials;
	}

	inline size_t materialCount() const {
		return m_materials.size();
	}

	inline void clearMaterials() {
		m_materials.clear();
	}

	//
	// mesh render
	//
	virtual void render(RenderContext* context) override;

protected:
	void loadEmbededMaterials();
	
protected:
	std::weak_ptr<Model> m_meshes;
	std::vector<std::weak_ptr<IMaterial>> m_materials;
};  