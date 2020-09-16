#pragma once
#include"Component.h"
#include"Material.h"
#include"MeshGroup.h"
#include"RendererCore.h"

class MeshRenderComponent : public Component {
public:
	MeshRenderComponent(std::shared_ptr<MeshGroup> meshes = nullptr, bool useEmbededMaterial = true);
	MeshRenderComponent(MeshRenderComponent&& rv) = default;
	MeshRenderComponent& operator = (MeshRenderComponent&& rv) = default;

	MeshRenderComponent& operator = (const MeshRenderComponent& other) = delete;
	MeshRenderComponent(const MeshRenderComponent& other) = delete;

	static const std::string s_identifier;

	inline std::string identifier() const override {
		return s_identifier;
	}

	Component* copy() const override;
	static MeshRenderComponent* create();
	static void destory(MeshRenderComponent* mrc);

	//
	// mesh managment
	//
	void setMeshes(std::shared_ptr<MeshGroup> meshes, bool useEmbededMaterials = true);
	size_t meshCount() const;
	
	inline std::shared_ptr<MeshGroup> getMeshes() const {
		return m_meshes;
	}	
	
	//
	//material managment
	//
	void addMaterial(std::shared_ptr<Material> m);
	void setMaterialAt(size_t index, std::shared_ptr<Material> m);
	std::shared_ptr<Material> materialAt(size_t index) const;
	void swapMaterial(size_t formIndex, size_t toIndex);
	void moveMaterial(size_t srcIndex, size_t dstIndex);

	inline const std::vector<std::shared_ptr<Material>>& materials() const {
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
	std::shared_ptr<MeshGroup> m_meshes;
	std::vector<std::shared_ptr<Material>> m_materials;
};  