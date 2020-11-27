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


void FrameBuffer::bindDefault(Target target) {
	GLCALL(glBindFramebuffer(int(target), 0));
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


FrameBuffer::Status FrameBuffer::checkStatus() const {
	GLCALL(int result = glCheckNamedFramebufferStatus(m_handler, GL_FRAMEBUFFER));
	return  Status(result);
}


void FrameBuffer::setDrawBufferLocation(const std::vector<int>& locations) {
	if (locations.empty()) {
		GLCALL(glNamedFramebufferDrawBuffer(m_handler, GL_NONE));
		return;
	}

	std::vector<GLenum> drawTargets;
	drawTargets.reserve(locations.size());

	for (auto i : locations) {
		drawTargets.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	GLCALL(glNamedFramebufferDrawBuffers(m_handler, drawTargets.size(), drawTargets.data()));
}


void FrameBuffer::setReadBufferLocation(int location) {
	if (location == -1) {
		GLCALL(glNamedFramebufferReadBuffer(m_handler, GL_NONE));
		return;
	}
	GLCALL(glNamedFramebufferReadBuffer(m_handler, int(GL_COLOR_ATTACHMENT0 + location)));
}

void FrameBuffer::copyData(unsigned int dstHandler, FrameBuffer::CopyFiled filedMask, int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1, bool usingLinearFilter) const {
	GLenum filter = usingLinearFilter ? GL_LINEAR : GL_NEAREST;
	GLCALL(glBlitNamedFramebuffer(m_handler, dstHandler, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, int(filedMask), filter));
}

void FrameBuffer::getDimension(int* width, int* height) const {
	int w = 0;
	int h = 0;
	GLCALL(glGetNamedFramebufferParameteriv(m_handler, GL_FRAMEBUFFER_DEFAULT_WIDTH, &w));
	GLCALL(glGetNamedFramebufferParameteriv(m_handler, GL_FRAMEBUFFER_DEFAULT_HEIGHT, &h));
	
	if (width)
		*width = w;
	if (height)
		*height = h;
}


void FrameBuffer::release() {
	if (m_handler) {
		if (m_bindTarget != Target::Unknown) 
			unbind();

		glDeleteFramebuffers(1, &m_handler);
		m_handler = 0;
		m_bindTarget = Target::Unknown;
	}
}