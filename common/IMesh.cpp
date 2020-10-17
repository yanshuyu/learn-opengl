#include"IMesh.h"
#include"Buffer.h"
#include"VertexArray.h"

IMesh::IMesh() :m_name()
, m_indices()
, m_primitiveType(PrimitiveType::Unknown)
, m_transform(1.f)
, m_vao()
, m_vbo()
, m_ibo() {
	m_id = reinterpret_cast<ID>(this);
}


IMesh::~IMesh() {
	release();
}


void IMesh::release() {
	m_name.clear();
	m_indices.clear();
	m_primitiveType = PrimitiveType::Unknown;
	m_transform = glm::mat4(1.f);
	m_vbo.release();
	m_ibo.release();
	m_vao.release();
}



std::ostream& operator << (std::ostream& o, const IMesh* mesh) {
	o << "{name: " << mesh->getName() << "(" << mesh->id() << ")" << ", vertexCount: " << mesh->verticesCount()
		<< ", indexCount: " << mesh->indicesCount() << ", primitive: " << toStr(mesh->getPrimitiveType()) << "}";
	return o;
}