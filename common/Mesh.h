#pragma once
#include"IMesh.h"
#include<glm/glm.hpp>
#include<glad/glad.h>


template<typename Vertex>
class TMesh: public IMesh {
	friend class Model;
	friend class MeshManager;

public:
	TMesh();
	TMesh(const TMesh& other) = delete;
	TMesh(TMesh&& other) noexcept;

	TMesh& operator = (const TMesh& other) = delete;
	TMesh& operator = (TMesh&& other) noexcept;

	void fill(const std::vector<Vertex>& vertices, const std::vector<Index_t>& indices);
	void fill(std::vector<Vertex>&& vertices, std::vector<Index_t>&& indices);
	void release() override;

	inline size_t verticesCount() const override {
		return m_vertics.size();
	}
	
	inline const std::vector<Vertex>& getVertices() const {
		return m_vertics;
	}

protected:
	void genGpuResources();

protected:
	std::vector<Vertex> m_vertics;
};


typedef TMesh<Vertex_t> Mesh;
typedef TMesh<SkinVertex_t> SkinMesh;
