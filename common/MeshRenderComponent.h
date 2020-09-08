#pragma once
#include"Component.h"
#include"Material.h"
#include"MeshGroup.h"
#include"RendererCore.h"

class MeshRenderComponent : public Component {
public:
	MeshRenderComponent(std::shared_ptr<MeshGroup> meshes = nullptr);
	MeshRenderComponent(MeshRenderComponent&& rv) = default;
	MeshRenderComponent& operator = (MeshRenderComponent&& rv) = default;

	MeshRenderComponent& operator = (const MeshRenderComponent& other) = delete;
	MeshRenderComponent(const MeshRenderComponent& other) = delete;

	static const std::string s_identifier;

	inline std::string indentifier() const override {
		return s_identifier;
	}

	Component* copy() const override;

	//
	// mesh managment
	//
	void setMeshes(std::shared_ptr<MeshGroup> meshes);
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
	void render(RenderContext* context) const;

private:
	void loadDefaultMaterials(MeshGroup* meshes);
	
private:
	std::shared_ptr<MeshGroup> m_meshes;
	std::vector<std::shared_ptr<Material>> m_materials;
};  