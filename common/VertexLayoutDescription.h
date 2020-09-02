#pragma once
#include<glad/glad.h>
#include<vector>

class VertexLayoutDescription {
public:
	enum class AttributeElementType {
		BYTE = GL_BYTE,
		UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
		SHORT = GL_SHORT,
		UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
		INT = GL_INT,
		UNSIGNED_INT = GL_UNSIGNED_INT,
		HALF_FLOAT = GL_HALF_FLOAT,
		FLOAT = GL_FLOAT,
		DOUBLE = GL_DOUBLE,
		FIXED = GL_FIXED,
	};

	struct VertexAttribute {
		VertexLayoutDescription::AttributeElementType elementType;
		size_t elementCount;
		size_t packedSize;
		bool normalized;

		VertexAttribute();
		VertexAttribute(VertexLayoutDescription::AttributeElementType et, 
																size_t ec, 
																size_t ps,  
																bool nor);
	};


public:
	VertexLayoutDescription(size_t stride = 0);
	
	void pushAttribute(AttributeElementType et, size_t ec, size_t packedSz = 0, bool normalize = false);
	
	void reset();

	size_t getStride() const;

	inline void setStride(size_t stride) { m_stride = stride; }

	inline const std::vector<VertexAttribute>& getAttributes() const {
		return m_attributes;
	}

private:
	size_t getAttributeElmentSize(AttributeElementType e);
private:
	size_t m_stride;
	std::vector<VertexAttribute> m_attributes;
};