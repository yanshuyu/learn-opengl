#include"VertexLayoutDescription.h"
#include"Util.h"
#include<algorithm>

VertexLayoutDescription::VertexAttribute::VertexAttribute():VertexAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 0, 0, false) {

}


VertexLayoutDescription::VertexAttribute::VertexAttribute(VertexLayoutDescription::AttributeElementType et,
																								size_t ec,
																								size_t ps,
																								bool nor) {
	elementType = et;
	elementCount = ec;
	packedSize = ps;
	normalized = nor;
}

VertexLayoutDescription::VertexLayoutDescription(size_t stride) :m_stride(stride) {

}


void VertexLayoutDescription::pushAttribute(AttributeElementType et, size_t ec, size_t packedSz, bool normalize) {
	if (packedSz <= 0) {
		packedSz = ec * getAttributeElmentSize(et);
	}
	m_attributes.push_back(VertexAttribute(et, ec, packedSz, normalize));
}


void VertexLayoutDescription::reset() {
	m_attributes.clear();
	m_stride = 0;
}

size_t VertexLayoutDescription::getStride() const {
	if (m_stride)
		return m_stride;

	size_t stride = 0;
	std::for_each(m_attributes.begin(), m_attributes.end(), [&](const VertexAttribute& va) {
		stride += va.packedSize;
	});

	return stride;
}


size_t VertexLayoutDescription::getAttributeElmentSize(AttributeElementType e) {
	switch (e)
	{
	case VertexLayoutDescription::AttributeElementType::BYTE:
		return sizeof(GLbyte);

	case VertexLayoutDescription::AttributeElementType::UNSIGNED_BYTE:
		return sizeof(GLubyte);

	case VertexLayoutDescription::AttributeElementType::SHORT:
		return sizeof(GLshort);

	case VertexLayoutDescription::AttributeElementType::UNSIGNED_SHORT:
		return sizeof(GLushort);

	case VertexLayoutDescription::AttributeElementType::INT:
		return sizeof(GLint);

	case VertexLayoutDescription::AttributeElementType::UNSIGNED_INT:
		return sizeof(GLuint);

	case VertexLayoutDescription::AttributeElementType::HALF_FLOAT:
		return sizeof(GLhalf);

	case VertexLayoutDescription::AttributeElementType::FLOAT:
		return sizeof(GLfloat);

	case VertexLayoutDescription::AttributeElementType::DOUBLE:
		return sizeof(GLdouble);

	case VertexLayoutDescription::AttributeElementType::FIXED:
		return sizeof(GLfixed);

	default:
		ASSERT(false);
	}
}