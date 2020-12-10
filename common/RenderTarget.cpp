#include"RenderTarget.h"

RenderTarget::RenderTarget(const glm::vec2& renderSz)
: m_renderSize(renderSz)
, m_frameBuffer()
, m_deptpStencilAttachments()
, m_colorAttachments() {

}

RenderTarget::~RenderTarget() {
}



bool RenderTarget::attachTexture2D(Texture::Format fmt, Slot slot, size_t index) {
	if (_hasAttactment(slot, index))
		return false;

	auto tex = std::unique_ptr<Texture>( new Texture());
	tex->allocStorage2D(fmt, m_renderSize.x, m_renderSize.y);
	
	return _attachTexture(std::move(tex), slot, index);
}


bool RenderTarget::attchTexture2DArray(Texture::Format fmt, size_t numLayer, Slot slot, size_t index) {
	if (_hasAttactment(slot, index))
		return false;

	auto tex = std::unique_ptr<Texture>(new Texture);
	tex->allocStorage2DArray(fmt, numLayer, m_renderSize.x, m_renderSize.y);

	return _attachTexture(std::move(tex), slot, index);
}


bool RenderTarget::attachTextureCube(Texture::Format fmt, Slot slot, size_t index) {
	if (_hasAttactment(slot, index))
		return false;

	auto texCube = std::unique_ptr<Texture>(new Texture);
	texCube->allocStorageCube(fmt, m_renderSize.x, m_renderSize.y);

	return _attachTexture(std::move(texCube), slot, index);
}


bool RenderTarget::attachProxyTexture(Texture* texture, Slot slot, size_t index) {
	if (!texture)
		return false;

	if (_hasAttactment(slot, index))
		return false;

	if (!m_frameBuffer.addTextureAttachment(texture->getHandler(), slot, index))
		return false;

	if (slot == Slot::Color) {
		m_colorProxy.insert({ index, texture });
	}
	else {
		m_deptpStencilProxy.insert({ slot, texture });
	}

	return true;
}


void RenderTarget::detachTexture(Slot slot, size_t index) {
	if (!_hasAttactment(slot, index))
		return;

	if (slot == Slot::Color) {
		if (m_colorAttachments.find(index) != m_colorAttachments.end())
			m_colorAttachments.erase(index);

		if (m_colorProxy.find(index) != m_colorProxy.end())
			m_colorProxy.erase(index);
	}
	else {
		if (m_deptpStencilAttachments.find(slot) != m_deptpStencilAttachments.end())
			m_deptpStencilAttachments.erase(slot);
		
		if (m_deptpStencilProxy.find(slot) != m_deptpStencilProxy.end())
			m_deptpStencilProxy.erase(slot);
	}

	m_frameBuffer.addTextureAttachment(0, slot, index);
}



Texture* RenderTarget::getAttachedTexture(Slot slot, size_t index) {
	if (slot == Slot::Color) {
		auto pos1 = m_colorAttachments.find(index);
		auto pos2 = m_colorProxy.find(index);
		return pos1 == m_colorAttachments.end() ? pos2 == m_colorProxy.end() ? nullptr : pos2->second 
			: pos1->second.get();
	} 

	auto pos1 = m_deptpStencilAttachments.find(slot);
	auto pos2 = m_deptpStencilProxy.find(slot);

	return pos1 == m_deptpStencilAttachments.end() ? pos2 == m_deptpStencilProxy.end() ? nullptr : pos2->second
		: pos1->second.get();
}


bool RenderTarget::resize(const glm::vec2& sz) {
	m_renderSize = sz;

	for (auto& attachment : m_deptpStencilProxy) {
		m_frameBuffer.addTextureAttachment(0, attachment.first);
	}

	for (auto& attachment : m_colorProxy) {
		m_frameBuffer.addTextureAttachment(0, Slot::Color, attachment.first);
	}
	m_deptpStencilProxy.clear();
	m_colorProxy.clear();

	auto oldDepthStencilAttachments = std::move(m_deptpStencilAttachments);
	for (auto& attachment : oldDepthStencilAttachments) {
		auto& slot = attachment.first;
		auto& texture = attachment.second;
		if (texture->getLayerCount() > 1) {
			if (!attchTexture2DArray(texture->getFormat(), texture->getLayerCount(), slot))
				return false;
		} else {
			if (!attachTexture2D(texture->getFormat(), slot))
				return false;
		}
	}
	oldDepthStencilAttachments.clear();

	auto oldColorAttachments = std::move(m_colorAttachments);
	for (auto& attachment : oldColorAttachments) {
		auto idx = attachment.first;
		auto& texture = attachment.second;
		if (texture->getLayerCount() > 1) {
			if (!attchTexture2DArray(texture->getFormat(), texture->getLayerCount(), Slot::Color, idx))
				return false;
		} else {
			if (!attachTexture2D(texture->getFormat(), Slot::Color, idx))
				return false;
		}
	}
	oldColorAttachments.clear();


	return true;
}


void RenderTarget::detachAllTexture() {
	for (auto& attachment : m_deptpStencilAttachments) {
		m_frameBuffer.addTextureAttachment(0, attachment.first);
	}

	for (auto& attachment : m_colorAttachments) {
		m_frameBuffer.addTextureAttachment(0, Slot::Color, attachment.first);
	}

	for (auto& attachment : m_deptpStencilProxy) {
		m_frameBuffer.addTextureAttachment(0, attachment.first);
	}

	for (auto& attachment : m_colorProxy) {
		m_frameBuffer.addTextureAttachment(0, Slot::Color, attachment.first);
	}

	m_deptpStencilProxy.clear();
	m_deptpStencilAttachments.clear();
	m_colorAttachments.clear();
	m_colorProxy.clear();
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



bool RenderTarget::_hasAttactment(Slot slot, size_t index) {
	if (slot == Slot::Color)
		return m_colorAttachments.find(index) != m_colorAttachments.end() || m_colorProxy.find(index) != m_colorProxy.end();

	return m_deptpStencilAttachments.find(slot) != m_deptpStencilAttachments.end() || m_deptpStencilProxy.find(slot) != m_deptpStencilProxy.end();
}