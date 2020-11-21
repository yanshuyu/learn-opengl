#pragma once
#include"Util.h"
#include"RendererCore.h"
#include<memory>

class Buffer;
class VertexArray;

class IMesh {
public:
	IMesh();
	virtual ~IMesh();

	virtual size_t verticesCount() const = 0;
	virtual void release();

	inline ID id() const {
		return m_id;
	}

	inline size_t indicesCount() const {
		return m_indices.size();
	}

	inline const std::vector<Index_t>& getIndices() const {
		return m_indices;
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


protected:
	ID m_id;
	std::string m_name;

	PrimitiveType m_primitiveType;
	glm::mat4 m_transform; // transform mesh from reletive to parent to reletive to model space

	std::vector<Index_t> m_indices; // cpu data

	std::unique_ptr<Buffer> m_vbo; // gpu data
	std::unique_ptr<Buffer> m_ibo;
	std::unique_ptr<VertexArray> m_vao;
};


std::ostream& operator << (std::ostream& o, const IMesh* mesh);