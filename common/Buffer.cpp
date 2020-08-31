#include"Buffer.h"
#include"Util.h"



Buffer::Buffer(const void* data, size_t sz, GLenum target, GLenum usage):m_handler(0)
, m_size(sz)
, m_target(target)
, m_usage(usage) {
	GLCALL(glGenBuffers(1, &m_handler));
	GLCALL(glBindBuffer(target, m_handler));
	GLCALL(glBufferData(target, sz, data, usage));
	GLCALL(glBindBuffer(target, 0));
}

Buffer::~Buffer() {
	release();
}

void Buffer::bind() const {
	GLCALL(glBindBuffer(m_target, m_handler));
}

void Buffer::unbind() const {
	GLCALL(glBindBuffer(m_target, 0));
}

void* Buffer::map(Buffer::MapAccess access) {
	GLCALL(void* mapped = glMapBuffer(m_target, GLenum(access)));
	return mapped;
}

void* Buffer::mapRange(Buffer::MapBitFiled mbf, size_t offset, size_t len) {
	GLCALL(void* mapped = glMapBufferRange(m_target, offset, len, GLbitfield(mbf)));
	return mapped;
}

bool Buffer::unmap() {
	GLCALL(bool result = glUnmapBuffer(m_target) == GL_TRUE);
	return result;
}

void Buffer::release() {
	if (m_handler) {
		glDeleteBuffers(1, &m_handler);
		m_handler = 0;
		m_size = 0;
	}
}