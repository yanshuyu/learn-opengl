#pragma once
#include"Buffer.h"
#include"VertexLayoutDescription.h"




class VertexArray {
public:
	VertexArray();
	~VertexArray();

	VertexArray(const VertexArray& other) = delete;
	VertexArray& operator = (const VertexArray& other) = delete;

	void bind() const;
	void unbind() const;
	void storeVertexLayout(const VertexLayoutDescription& vbDesc);
	void release();

	inline unsigned int getHandler() const {
		return m_handler;
	}

	inline bool isVailde() const {
		return m_handler != 0;
	}

private:
	GLuint m_handler;
};