#include"Mesh.h"
#include"VertexArray.h"
#include"Buffer.h"


template TMesh<Vertex_t>;
template TMesh<SkinVertex_t>;


template<typename Vertex>
TMesh<Vertex>::TMesh() :IMesh()
, m_vertics() {
}

template<typename Vertex>
TMesh<Vertex>::TMesh(TMesh<Vertex>&& other)noexcept : IMesh() {
	*this = std::move(other);
}

template<typename Vertex>
TMesh<Vertex>& TMesh<Vertex>::operator = (TMesh<Vertex>&& other) noexcept {
	if (&other == this)
		return *this;

	m_name = std::move(other.m_name);
	m_vertics = std::move(other.m_vertics);
	m_indices = std::move(other.m_indices);
	m_primitiveType = other.m_primitiveType;
	m_transform = std::move(other.m_transform);
	m_vbo = std::move(other.m_vbo);
	m_ibo = std::move(other.m_ibo);
	m_vao = std::move(other.m_vao);
	
	return *this;
}

template<typename Vertex>
void TMesh<Vertex>::fill(const std::vector<Vertex>& vertices, const std::vector<Index_t>& indices) {
	m_vertics = vertices;
	m_indices = indices;
	genGpuResources();
}


template<typename Vertex>
void TMesh<Vertex>::fill(std::vector<Vertex>&& vertices, std::vector<Index_t>&& indices) {
	m_vertics = std::move(vertices);
	m_indices = std::move(indices);
	genGpuResources();
}


template<typename Vertex>
void TMesh<Vertex>::release() {
	m_vertics.clear();
	__super::release();
}


template<>
void TMesh<Vertex_t>::genGpuResources() {
#ifdef _DEBUG
	ASSERT(m_vertics.size() > 0);
#endif // _DEBUG

	m_vao = std::make_unique<VertexArray>();
	m_vbo = std::make_unique<Buffer>(m_vertics.data(), sizeof(Vertex_t) * m_vertics.size(), Buffer::Target::VertexBuffer, Buffer::Usage::StaticDraw, m_vertics.size());
	if (m_indices.size() > 0) {
		m_ibo = std::make_unique<Buffer>(m_indices.data(), sizeof(Index_t) * m_indices.size(), Buffer::Target::IndexBuffer, Buffer::Usage::StaticDraw, m_indices.size());
	}

	VertexLayoutDescription layoutDesc;
	layoutDesc.setStride(sizeof(Vertex_t));
	layoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 3, 0); // position
	layoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 3, 1); // normal
	layoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 3, 2); // tangent
	layoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 2, 3); // uv

	m_vao->bind();
	m_vbo->bind(Buffer::Target::VertexBuffer);
	if (m_ibo)
		m_ibo->bind(Buffer::Target::IndexBuffer);
	m_vao->storeVertexLayout(layoutDesc);
	m_vao->unbind();
	m_vbo->unbind();
	if (m_ibo)
		m_ibo->unbind();
}


template<>
void TMesh<SkinVertex_t>::genGpuResources() {
#ifdef _DEBUG
	ASSERT(m_vertics.size() > 0);
#endif // _DEBUG

	m_vao = std::make_unique<VertexArray>();
	m_vbo = std::make_unique<Buffer>(m_vertics.data(), sizeof(SkinVertex_t) * m_vertics.size(), Buffer::Target::VertexBuffer, Buffer::Usage::StaticDraw, m_vertics.size());
	if (m_indices.size() > 0) {
		m_ibo = std::make_unique<Buffer>(m_indices.data(), sizeof(Index_t) * m_indices.size(), Buffer::Target::IndexBuffer, Buffer::Usage::StaticDraw, m_indices.size());
	}

	VertexLayoutDescription layoutDesc;
	layoutDesc.setStride(sizeof(SkinVertex_t));
	layoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::INT, 4, 4, sizeof(GLint)*4); // joints
	layoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 4, 5, sizeof(GLfloat)*4); // weights
	layoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 3, 0, sizeof(GLfloat)*3); // position
	layoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 3, 1, sizeof(GLfloat)*3); // normal
	layoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 3, 2, sizeof(GLfloat)*3); // tangent
	layoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 2, 3); // uv


	m_vao->bind();
	m_vbo->bind(Buffer::Target::VertexBuffer);
	if (m_ibo)
		m_ibo->bind(Buffer::Target::IndexBuffer);
	m_vao->storeVertexLayout(layoutDesc);
	m_vao->unbind();
	m_vbo->unbind();
	if (m_ibo)
		m_ibo->unbind();
}

