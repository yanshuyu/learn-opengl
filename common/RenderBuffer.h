#pragma once
#include<glad/glad.h>
#include"Texture.h"



class RenderBuffer {

public:
	RenderBuffer();
	~RenderBuffer();

	void bind() const;
	void unbind() const;
	void allocStorage(Texture::Format fmt, size_t w, size_t h) const;
	void release();

	inline unsigned int getHandler() const {
		return m_handler;
	}

private:
	unsigned int m_handler;
};