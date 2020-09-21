#pragma once
#include<glad/glad.h>


class FrameBuffer {
public:
	enum class Target {
		Unknown,
		Read = GL_READ_FRAMEBUFFER,
		Draw = GL_DRAW_FRAMEBUFFER,
		ReadAndDraw = GL_FRAMEBUFFER,
	};
	

	enum class AttachmentPoint {
		Depth = GL_DEPTH_ATTACHMENT,
		Stencil = GL_STENCIL_ATTACHMENT,
		Depth_Stencil = GL_DEPTH_STENCIL_ATTACHMENT,
		Color = GL_COLOR_ATTACHMENT0,
	};


public:
	FrameBuffer();
	~FrameBuffer();

	FrameBuffer(const FrameBuffer& other) = delete;
	FrameBuffer(FrameBuffer&& rv) = delete;

	FrameBuffer& operator = (const FrameBuffer& other) = delete;
	FrameBuffer& operator = (FrameBuffer&& rv) = delete;

	static size_t maximumColorAttachment();

	void bind(Target target = Target::ReadAndDraw) const;
	void unbind() const;

	bool addTextureAttachment(unsigned int texHandler, AttachmentPoint ap, size_t index = 0) const;
	bool addRenderBufferAttachment(unsigned int bufHandler, AttachmentPoint ap, size_t index = 0) const;

	void release();

	inline unsigned int getHandler() const {
		return m_handler;
	}

	inline Target getBindTarget() const {
		return m_bindTarget;
	}


private:
	unsigned int  m_handler;
	mutable Target m_bindTarget;

};