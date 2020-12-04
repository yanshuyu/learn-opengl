#pragma once
#include"FrameBuffer.h"
#include"Texture.h"
#include<unordered_map>
#include<vector>


// render target manage frame buffer and associate resource
class RenderTarget {
public:
	typedef FrameBuffer::AttachmentPoint Slot; 
	typedef FrameBuffer::Target Target;
	typedef FrameBuffer::Status Status;

public:
	RenderTarget(const glm::vec2& renderSz);
	~RenderTarget();

	bool attachTexture2D(Texture::Format gpuGmt, Texture::Format cpuFmt,Texture::FormatDataType dataType, Slot slot, size_t index = 0);
	bool attchTexture2DArray(Texture::Format gpuFmt, Texture::Format cpuFmt, Texture::FormatDataType dataType, size_t numLayer, Slot slot, size_t index = 0);
	bool attachTextureCube(Texture::Format gpuFmt, Texture::Format cpuFmt, Texture::FormatDataType dataType, Slot slot, size_t index = 0);
	bool attachProxyTexture(Texture* texture, Slot slot, size_t index = 0);

	void detachTexture(Slot slot, size_t index = 0);
	void detachAllTexture();

	bool resize(const glm::vec2& sz);


	inline void bind(Target target = Target::ReadAndDraw) {
		m_frameBuffer.bind(target);
	}

	inline void unBind() {
		m_frameBuffer.unbind();
	}

	Texture* getAttachedTexture(Slot slot, size_t index = 0);

	inline void setDrawLocations(const std::vector<int>& locations) {
		m_frameBuffer.setDrawBufferLocation(locations);
	}

	inline Target getBindTarget() const {
		return m_frameBuffer.getBindTarget();
	}

	inline Status getStatus() const {
		return m_frameBuffer.checkStatus();
	}

	inline bool isValid() const {
		return m_frameBuffer.checkStatus() == Status::Ok;
	}

protected:
	bool _attachTexture(std::unique_ptr<Texture>&& tex, Slot slot, size_t index);
	bool _hasAttactment(Slot slot, size_t index);

protected:
	FrameBuffer m_frameBuffer;
	glm::vec2 m_renderSize;
	
	// attachments manage by this render target
	std::unordered_map<Slot, std::unique_ptr<Texture>> m_deptpStencilAttachments;
	std::unordered_map<size_t, std::unique_ptr<Texture>> m_colorAttachments;

	// attachments manage by external context
	std::unordered_map<Slot, Texture*> m_deptpStencilProxy;
	std::unordered_map<size_t, Texture*> m_colorProxy;
};
