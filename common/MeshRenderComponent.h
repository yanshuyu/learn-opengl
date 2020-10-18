#pragma once
#include"Component.h"
#include"Material.h"
#include"Model.h"
#include"RendererCore.h"

class MeshRenderComponent : public Component {
public:
	MeshRenderComponent(std::weak_ptr<Model> meshes = std::weak_ptr<Model>(), bool useEmbededMaterial = true);
	MeshRenderComponent(MeshRenderComponent&& rv) = default;
	MeshRenderComponent& operator = (MeshRenderComponent&& rv) = default;

	MeshRenderComponent& operator = (const MeshRenderComponent& other) = delete;
	MeshRenderComponent(const MeshRenderComponent& other) = delete;

	COMPONENT_IDENTIFIER_DEC;

	inline std::string identifier() const override {
		return s_identifier;
	}

	Component* copy() const override;
	static MeshRenderComponent* create();
	static void destory(MeshRenderComponent* mrc);

	//
	// mesh managment
	//
	void setMeshes(std::weak_ptr<Model> meshes, bool useEmbededMaterials = true);

	inline std::weak_ptr<Model> getMeshes() const {
		return m_meshes;
	}	
	
	//
	//material managment
	//
	void addMaterial(std::weak_ptr<Material> m);
	void setMaterialAt(size_t index, std::weak_ptr<Material> m);
	std::weak_ptr<Material> materialAt(size_t index) const;
	void swapMaterial(size_t formIndex, size_t toIndex);
	void moveMaterial(size_t srcIndex, size_t dstIndex);

	inline const std::vector<std::weak_ptr<Material>>& materials() const {
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
	void render(RenderContext* context) override;

private:
	void loadEmbededMaterials();
	
private:
	std::weak_ptr<Model> m_meshes;
	std::vector<std::weak_ptr<Material>> m_materials;
};  