#include"VertexArray.h"
#include"Util.h"


VertexArray::VertexArray():m_handler(0) {
	GLCALL(glGenVertexArrays(1, &m_handler));
}

VertexArray::~VertexArray() {
	release();
}

void VertexArray::bind() const {
	GLCALL(glBindVertexArray(m_handler));
}

void VertexArray::unbind() const {
	GLCALL(glBindVertexArray(0));
}

void VertexArray::storeVertexLayout(const VertexLayoutDescription& vbDesc) {
	bind();
	size_t stride = vbDesc.getStride();
	size_t offset = 0;
	auto attributes = vbDesc.getAttributes();
	for (size_t i = 0; i < attributes.size(); i++) {
		auto& a = attributes[i];
		GLCALL(glEnableVertexAttribArray(a.location));
		if (vbDesc.isIntegerAttribute(a.elementType)) {
			GLCALL(glVertexAttribIPointer(a.location, a.elementCount, 
				GLenum(a.elementType),
				stride, 
				reinterpret_cast<void*>(offset)));
		} else {
			GLCALL(glVertexAttribPointer(a.location,
				a.elementCount,
				GLenum(a.elementType),
				a.normalized,
				stride,
				reinterpret_cast<void*>(offset)));
		}
		offset += a.packedSize;
	}
}


void VertexArray::release() {
	if (m_handler != 0) {
		GLCALL(glDeleteVertexArrays(1, &m_handler));
		m_handler = 0;
	}
}
