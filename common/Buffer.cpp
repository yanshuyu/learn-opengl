#include"Buffer.h"
#include"Util.h"

Buffer::Buffer(): m_handler(0)
, m_size(0)
, m_elementCount(0)
, m_target(Target::Unknown)
, m_usage(Usage::Unknown) {
	GLCALL(glGenBuffers(1, &m_handler));
}

Buffer::Buffer(const void* data, size_t dataSz, Target target, Usage usage, size_t elementCnt) : Buffer() {
	bind(target);
	loadData(data, dataSz, usage, elementCnt);
}

Buffer::~Buffer() {
	release();
}

void Buffer::bind(Target target) const {
	GLCALL(glBindBuffer(int(target), m_handler));
	m_target = target;
}

void Buffer::unbind() const {
	if (m_target != Target::Unknown) {
		GLCALL(glBindBuffer(int(m_target), 0));
		m_target = Target::Unknown;
	}
}

void Buffer::loadData(const void* data, size_t dataSz, Usage usage, size_t elementCnt) {
	GLCALL(glBufferData(int(m_target), dataSz, data, int(usage)));
	m_usage = usage;
	m_size = dataSz;
	m_elementCount = elementCnt;
}

void* Buffer::map(Buffer::MapAccess access) {
	GLCALL(void* mapped = glMapBuffer(int(m_target), GLenum(access)));
	return mapped;
}

void* Buffer::mapRange(Buffer::MapBitFiled mbf, size_t offset, size_t len) {
	GLCALL(void* mapped = glMapBufferRange(int(m_target), offset, len, GLbitfield(mbf)));
	return mapped;
}

bool Buffer::unmap() {
	GLCALL(bool result = glUnmapBuffer(int(m_target)) == GL_TRUE);
	return result;
}

void Buffer::release() {
	if (m_handler) {
		unbind();
		glDeleteBuffers(1, &m_handler);
		m_handler = 0;
		m_size = 0;
		m_elementCount = 0;
		m_target = Target::Unknown;
		m_usage = Usage::Unknown;
	}
}