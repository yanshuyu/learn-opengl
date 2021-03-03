#pragma once
#include<glad/glad.h>
#include<vector>

class FrameBuffer {
public:
	enum class Target {
		Unknown,
		Read = GL_READ_FRAMEBUFFER,
		Draw = GL_DRAW_FRAMEBUFFER,
		ReadAndDraw = GL_FRAMEBUFFER,
	};
	

	enum class Slot {
		Depth = GL_DEPTH_ATTACHMENT,
		Stencil = GL_STENCIL_ATTACHMENT,
		Depth_Stencil = GL_DEPTH_STENCIL_ATTACHMENT,
		Color = GL_COLOR_ATTACHMENT0,
	};

	enum class Status {
		Error = 0,
		Ok = GL_FRAMEBUFFER_COMPLETE,
		Undefine = GL_FRAMEBUFFER_UNDEFINED,
		InComplete_Attachment = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
		Missing_Image_Attachment = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
		Incomplete_Draw_Buffer = GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
		Incomplete_Read_Buffer = GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
		Unsupported = GL_FRAMEBUFFER_UNSUPPORTED, // combination of internal formats of the attached images unsupport
		Incompelet_Multisample = GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
		Incomplete_Layer_Target = GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS,

	};


	enum CopyFiled {
		Color = GL_COLOR_BUFFER_BIT,
		Depth = GL_DEPTH_BUFFER_BIT,
		Stencil = GL_STENCIL_BUFFER_BIT,
	};


public:
	FrameBuffer();
	~FrameBuffer();

	FrameBuffer(const FrameBuffer& other) = delete;
	FrameBuffer(FrameBuffer&& rv) = delete;
	FrameBuffer& operator = (const FrameBuffer& other) = delete;
	FrameBuffer& operator = (FrameBuffer&& rv) = delete;

	static size_t maximumColorAttachment();
	static void bindDefault(Target target = Target::ReadAndDraw);

	void bind(Target target = Target::ReadAndDraw) const;
	void unbind() const;

	bool addTextureAttachment(unsigned int texHandler, Slot slot, size_t index = 0) const;
	bool addRenderBufferAttachment(unsigned int bufHandler, Slot slot, size_t index = 0) const;
	Status checkStatus() const;
	void release();

	void setDrawBufferLocation(const std::vector<int>& locations);  // shader output location map to buffer attachment	
	void setReadBufferLocation(int location);
	bool copyData(unsigned int dstHandler, CopyFiled filedMask, int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1, bool usingLinearFilter = true) const; //  copy a block of pixels from one framebuffer object to another

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