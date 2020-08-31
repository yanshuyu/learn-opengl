#pragma once
#include"Buffer.h"
#include"VertexLayoutDescription.h"




class VertexArray {
public:
	VertexArray();
	~VertexArray();

	void bind() const;
	void unbind() const;
	void storeVertexLayout(const Buffer& vb, const VertexLayoutDescription& vbDesc);
	void release();

	inline unsigned int getHandler() const {
		return m_handler;
	}

private:
	GLuint m_handler;
};