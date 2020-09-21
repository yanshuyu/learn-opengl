#include"RenderBuffer.h"
#include"Util.h"

RenderBuffer::RenderBuffer() :m_handler(0) {

}

RenderBuffer::~RenderBuffer() {
	release();
}

void RenderBuffer::bind() const {
	GLCALL(glBindRenderbuffer(GL_RENDERBUFFER, m_handler));
}

void RenderBuffer::unbind() const {
	GLCALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
}

void RenderBuffer::allocStorage(Texture::Format fmt, size_t w, size_t h) const {
	GLCALL(glNamedRenderbufferStorage(m_handler, int(fmt), w, h));
}

void RenderBuffer::release() {
	if (m_handler) {
		GLCALL(glDeleteRenderbuffers(1, &m_handler));
		m_handler = 0;
	}
}
