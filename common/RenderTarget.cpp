#include"RenderTarget.h"

RenderTarget::RenderTarget(const glm::vec2& renderSz)
: m_renderSize(renderSz)
, m_frameBuffer()
, m_deptpStencilAttachments()
, m_colorAttachments() {

}

RenderTarget::~RenderTarget() {
}



bool RenderTarget::attachTexture2D(Texture::Format gpuFmt, Texture::Format cpuFmt, Texture::FormatDataType dataType, Slot slot, size_t index) {
	auto tex = std::unique_ptr<Texture>( new Texture());
	tex->bind();

	if (!tex->loadImage2DFromMemory(gpuFmt, cpuFmt, dataType, m_renderSize.x, m_renderSize.y, nullptr))
		return false;
	
	tex->unbind();
	
	return _attachTexture(std::move(tex), slot, index);
}


bool RenderTarget::attchTexture2DArray(Texture::Format gpuFmt, Texture::Format cpuFmt, Texture::FormatDataType dataType, size_t numLayer, Slot slot, size_t index) {
	auto tex = std::unique_ptr<Texture>(new Texture);
	tex->bind(Texture::Unit::Defualt, Texture::Target::Texture_2D_Array);

	if (!tex->loadImage2DArrayFromMemory(gpuFmt, cpuFmt, dataType, m_renderSize.x, m_renderSize.y, numLayer, nullptr))
		return false;

	tex->unbind();

	return _attachTexture(std::move(tex), slot, index);
}


bool RenderTarget::attachTextureCube(Texture::Format gpuFmt, Texture::Format cpuFmt, Texture::FormatDataType dataType, Slot slot, size_t index) {
	auto cube = std::unique_ptr<Texture>(new Texture);
	cube->bind(Texture::Unit::Defualt, Texture::Target::Texture_CubeMap);

	if (!cube->loadCubeMapFromMemory(gpuFmt, cpuFmt, dataType, m_renderSize.x, m_renderSize.y))
		return false;

	cube->unbind();

	return _attachTexture(std::move(cube), slot, index);
}



Texture* RenderTarget::getAttachedTexture(Slot slot, size_t index) {
	if (slot == Slot::Color) {
		auto pos = m_colorAttachments.find(index);
		return pos == m_colorAttachments.end() ? nullptr : pos->second.get();
	} 

	auto pos = m_deptpStencilAttachments.find(slot);
	return pos == m_deptpStencilAttachments.end() ? nullptr : pos->second.get();
}


bool RenderTarget::resize(const glm::vec2& sz) {
	m_renderSize = sz;

	if (m_deptpStencilAttachments.empty() && m_colorAttachments.empty())
		return true;

	std::unordered_map<Slot, std::unique_ptr<Texture>> oldDepthStencilAttachments = std::move(m_deptpStencilAttachments);
	for (auto& attachment : oldDepthStencilAttachments) {
		auto& slot = attachment.first;
		auto& texture = attachment.second;
		if (texture->getLayerCount() > 1) {
			if (!attchTexture2DArray(texture->getFormat(), texture->getCpuFormat(), texture->getCpuFormatDataType(), texture->getLayerCount(), slot))
				return false;
		} else {
			if (!attachTexture2D(texture->getFormat(), texture->getCpuFormat(), texture->getCpuFormatDataType(), slot))
				return false;
		}
	}
	oldDepthStencilAttachments.clear();

	std::unordered_map<size_t, std::unique_ptr<Texture>> oldColorAttachments = std::move(m_colorAttachments);
	for (auto& attachment : oldColorAttachments) {
		auto idx = attachment.first;
		auto& texture = attachment.second;
		if (texture->getLayerCount() > 1) {
			if (!attchTexture2DArray(texture->getFormat(), texture->getCpuFormat(), texture->getCpuFormatDataType(), texture->getLayerCount(), Slot::Color, idx))
				return false;
		} else {
			if (!attachTexture2D(texture->getFormat(), texture->getCpuFormat(), texture->getCpuFormatDataType(), Slot::Color, idx))
				return false;
		}
	}
	oldColorAttachments.clear();

	return true;
}


void RenderTarget::cleanUp() {
	for (auto& attachment : m_deptpStencilAttachments) {
		m_frameBuffer.addTextureAttachment(0, attachment.first);
	}

	for (auto& attachment : m_colorAttachments) {
		m_frameBuffer.addTextureAttachment(0, Slot::Color, attachment.first);
	}

	m_deptpStencilAttachments.clear();
	m_colorAttachments.clear();
}


bool RenderTarget::_attachTexture(std::unique_ptr<Texture>&& tex, Slot slot, size_t index) {
	if (!m_frameBuffer.addTextureAttachment(tex->getHandler(), slot, index))
		return false;

	if (slot == Slot::Color) {
		m_colorAttachments[index] = std::move(tex);
	}
	else {
		m_deptpStencilAttachments[slot] = std::move(tex);
	}

	return true;
}