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

bool FrameBuffer::addTextureAttachment(unsigned int texHandler, Slot slot, size_t index) const {
	if (m_bindTarget == Target::Unknown)
		return false;

	int attachment = slot == Slot::Color ? int(slot) + index : int(slot);
	//GLCALL(glNamedFramebufferTexture(m_handler, attachment, texHandler, 0)); // opengl 450
	GLCALL(glFramebufferTexture(GLenum(m_bindTarget), GLenum(attachment), texHandler, 0));
}


bool FrameBuffer::addRenderBufferAttachment(unsigned int bufHandler, Slot slot, size_t index) const {
	if (m_bindTarget == Target::Unknown)
		return false;

	int attachment = slot == Slot::Color ? int(slot) + index : int(slot);
	//GLCALL(glNamedFramebufferRenderbuffer(m_handler, attachment, GL_RENDERBUFFER, bufHandler));
	GLCALL(glFramebufferRenderbuffer(GLenum(m_bindTarget), GLenum(slot), GL_RENDERBUFFER, bufHandler));
}


FrameBuffer::Status FrameBuffer::checkStatus() const {
	//GLCALL(int result = glCheckNamedFramebufferStatus(m_handler, GL_FRAMEBUFFER));
	if (m_bindTarget == Target::Unknown)
		return Status::Undefine;

	GLCALL(int result = glCheckFramebufferStatus(GLenum(m_bindTarget)));
	return  Status(result);
}


void FrameBuffer::setDrawBufferLocation(const std::vector<int>& locations) {
	if (m_bindTarget == Target::Unknown)
		return;

	if (locations.empty()) {
		//GLCALL(glNamedFramebufferDrawBuffer(m_handler, GL_NONE));
		GLCALL(glDrawBuffer(GL_NONE));
		return;
	}

	std::vector<GLenum> drawTargets;
	drawTargets.reserve(locations.size());

	for (auto i : locations) {
		drawTargets.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	//GLCALL(glNamedFramebufferDrawBuffers(m_handler, drawTargets.size(), drawTargets.data()));
	GLCALL(glDrawBuffers(drawTargets.size(), drawTargets.data()));
}


void FrameBuffer::setReadBufferLocation(int location) {
	if (m_bindTarget == Target::Unknown)
		return;

	if (location < 0) {
		//GLCALL(glNamedFramebufferReadBuffer(m_handler, GL_NONE));
		GLCALL(glReadBuffer(GL_NONE));
		return;
	}
	
	//GLCALL(glNamedFramebufferReadBuffer(m_handler, int(GL_COLOR_ATTACHMENT0 + location)));
	GLCALL(glReadBuffer(int(GL_COLOR_ATTACHMENT0 + location)));
}

bool FrameBuffer::copyData(unsigned int dstHandler, FrameBuffer::CopyFiled filedMask, int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1, bool usingLinearFilter) const {
	if (m_bindTarget != Target::Read)
		return false;

	GLenum filter = usingLinearFilter ? GL_LINEAR : GL_NEAREST;
	//GLCALL(glBlitNamedFramebuffer(m_handler, dstHandler, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, int(filedMask), filter));
	GLCALL(glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, int(filedMask), filter));
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