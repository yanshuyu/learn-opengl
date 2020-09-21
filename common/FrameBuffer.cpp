#include"FrameBuffer.h"
#include"Util.h"


FrameBuffer::FrameBuffer() :m_handler(0)
, m_bindTarget(Target::Unknown) {
	GLCALL(glGenFramebuffers(1, &m_handler));
}


FrameBuffer::~FrameBuffer() {
	release();
}


size_t FrameBuffer::maximumColorAttachment() {
	int result = 0;
	GLCALL(glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &result));
	return result;
}


void FrameBuffer::bind(Target target) const {
	GLCALL(glBindFramebuffer(int(target), m_handler));
	m_bindTarget = target;
}


void FrameBuffer::unbind() const {
	if (m_bindTarget != Target::Unknown) {
		glBindFramebuffer(int(m_bindTarget), 0);
		m_bindTarget = Target::Unknown;
	}
}

bool FrameBuffer::addTextureAttachment(unsigned int texHandler, AttachmentPoint ap, size_t index) const {
	if (ap == AttachmentPoint::Color) {
		if (index >= maximumColorAttachment()) {
#ifdef _DEBUG
			ASSERT(false);
#endif // _DEBUG
			return false;
		}
	}

	int attachment = ap == AttachmentPoint::Color ? int(ap) + index : int(ap);
	GLCALL(glNamedFramebufferTexture(m_handler, attachment, texHandler, 0));
}


bool FrameBuffer::addRenderBufferAttachment(unsigned int bufHandler, AttachmentPoint ap, size_t index) const {
	if (ap == AttachmentPoint::Color) {
		if (index >= maximumColorAttachment()) {
#ifdef _DEBUG
			ASSERT(false);
#endif // _DEBUG
			return false;
		}
	}

	int attachment = ap == AttachmentPoint::Color ? int(ap) + index : int(ap);
	GLCALL(glNamedFramebufferRenderbuffer(m_handler, attachment, GL_RENDERBUFFER, bufHandler));
}


void FrameBuffer::release() {
	if (m_handler) {
		unbind();
		glDeleteFramebuffers(1, &m_handler);
		m_handler = 0;
		m_bindTarget = Target::Unknown;
	}
}