#include"Mesh.h"
#include"Util.h"

Mesh::Mesh() :m_name()
, m_primitiveType(Mesh::PrimitiveType::Triangle)
, m_vertics()
, m_indices()
, m_transform(1)
, m_vbo(nullptr)
, m_ibo(nullptr)
, m_vao(nullptr)
, m_id(0) {
	m_id = reinterpret_cast<unsigned long>(this);
}


Mesh::Mesh(Mesh&& other)noexcept : m_name(std::move(other.m_name))
, m_primitiveType(other.m_primitiveType)
, m_vertics(std::move(other.m_vertics))
, m_indices(std::move(other.m_indices))
, m_transform(std::move(other.m_transform))
, m_vbo(std::move(other.m_vbo))
, m_ibo(std::move(other.m_ibo))
, m_vao(std::move(other.m_vao))
, m_id(0) {
	m_id = reinterpret_cast<unsigned long>(this);
}


Mesh::~Mesh() {
	release();
}


Mesh& Mesh::operator = (Mesh&& other) noexcept {
	m_name = std::move(other.m_name);
	m_primitiveType = other.m_primitiveType;
	m_vertics = std::move(other.m_vertics);
	m_indices = std::move(other.m_indices);
	m_transform = std::move(other.m_transform);
	m_vbo = std::move(other.m_vbo);
	m_ibo = std::move(other.m_ibo);
	m_vao = std::move(other.m_vao);
	
	return *this;
}


void Mesh::fill(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, PrimitiveType pt) {
	release();
	m_vertics = vertices;
	m_indices = indices;
	m_primitiveType = pt;
	genRenderBuffers();
}

void Mesh::fill(std::vector<Vertex>&& vertices, std::vector<Index>&& indices, PrimitiveType pt) {
	release();
	m_vertics = std::move(vertices);
	m_indices = std::move(indices);
	m_primitiveType = pt;
	genRenderBuffers();
}

void Mesh::genRenderBuffers() {
	ASSERT(m_vertics.size() > 0);
	m_vao = std::make_unique<VertexArray>();
	m_vbo = std::make_unique<Buffer>(m_vertics.data(), sizeof(Vertex) * m_vertics.size(), GL_ARRAY_BUFFER, GL_STATIC_DRAW, m_vertics.size());
	if (m_indices.size() > 0) {
		m_ibo = std::make_unique<Buffer>(m_indices.data(), sizeof(Index) * m_indices.size(), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, m_indices.size());
	}
	
	VertexLayoutDescription layoutDesc;
	layoutDesc.setStride(sizeof(Vertex));
	layoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 3); // position
	layoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 3); // normal
	layoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 3); // tangent
	layoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 3); // bitangent
	layoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 2); // uv

	m_vao->bind();
	m_vbo->bind();
	if (m_ibo) m_ibo->bind();
	m_vao->storeVertexLayout(layoutDesc);
	m_vao->unbind();
	m_vbo->unbind();
	if(m_ibo) m_ibo->unbind();
}


void Mesh::release() {
	m_name.clear();
	m_vertics.clear();
	m_indices.clear();
	m_primitiveType = PrimitiveType::Triangle;

	m_vao = nullptr;
	m_vbo = nullptr;
	m_ibo = nullptr;
}


std::ostream& operator << (std::ostream& o, const Mesh& mesh) {
	o << "{name: " << mesh.getName() << "(" << mesh.id() << ")" << ", vertexCount: " << mesh.verticesCount()
		<< ", indexCount" << mesh.indicesCount() << ", primitive: " << int(mesh.getPrimitiveType()) << "}";
	return o;
}
