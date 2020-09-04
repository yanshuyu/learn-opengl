#pragma once
#include<glm/glm.hpp>
#include<glad/glad.h>
#include"Buffer.h"
#include"VertexArray.h"
#include<vector>
#include<string>
#include<memory>

class MeshGroup;
class MeshManager;


typedef unsigned int Index;


struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 biTangent;
	glm::vec2 uv;
};




class Mesh {
	friend class MeshGroup;
	friend class MeshManager;
public:
	enum class PrimitiveType {
		Point,
		Line,
		Triangle,
		Polygon,
		Unknown,
	};

public:
	Mesh();
	Mesh(const Mesh& other) = delete;
	Mesh(Mesh&& other) noexcept;
	~Mesh();

	Mesh& operator = (const Mesh& other) = delete;
	Mesh& operator = (Mesh&& other) noexcept;


	void fill(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, PrimitiveType pt = PrimitiveType::Unknown);
	void fill(std::vector<Vertex>&& vertices, std::vector<Index>&& indices, PrimitiveType pt = PrimitiveType::Unknown);
	void release();

	inline unsigned long id() const {
		return m_id;
	}

	inline size_t verticesCount() const {
		return m_vertics.size();
	}

	inline size_t indicesCount() const {
		return m_indices.size();
	}

	inline VertexArray* vertexArray() const {
		return m_vao.get();
	}

	inline void setName(const std::string& name) {
		m_name = name;
	}

	inline std::string getName() const {
		return m_name;
	}

	inline void setPrimitiveType(PrimitiveType pt) {
		m_primitiveType = pt;
	}

	inline PrimitiveType getPrimitiveType() const {
		return m_primitiveType;
	}

	inline void setTransform(const glm::mat4& t) {
		m_transform = t;
	}

	inline glm::mat4 getTransform() const {
		return m_transform;
	}
	
	inline const std::vector<Vertex>& getVertices() const {
		return m_vertics;
	}

	inline const std::vector<Index>& getIndices() const {
		return m_indices;
	}

private:
	void genRenderBuffers();

private:
	PrimitiveType m_primitiveType;
	unsigned long m_id;
	std::string m_name;
	std::vector<Vertex> m_vertics;
	std::vector<Index> m_indices;
	glm::mat4 m_transform;

	std::unique_ptr<Buffer> m_vbo;
	std::unique_ptr<Buffer> m_ibo;
	std::unique_ptr<VertexArray> m_vao;
};



std::ostream& operator << (std::ostream& o, const Mesh& mesh);