#include"TextureMgr.h"


std::shared_ptr<Texture> TextureManager::addTexture(const std::string& name, const std::string& file) {
	auto texture = getTexture(name);
	if (texture != nullptr)
		return texture;
	
	texture = std::make_shared<Texture>(file);
	
	if (!texture->load())
		return nullptr;

	m_textures.insert(std::make_pair(name, texture));
	
	return texture;
}

std::shared_ptr<Texture> TextureManager::getTexture(const std::string& name) const {
	auto pos = m_textures.find(name);
	if (pos != m_textures.end())
		return pos->second;
	
	return nullptr;
}


std::shared_ptr<Texture> TextureManager::removeTexture(const std::string& name) {
	auto pos = m_textures.find(name);
	if (pos != m_textures.end()) {
		m_textures.erase(pos);
		return pos->second;
	}

	return nullptr;
}


bool TextureManager::hasTexture(const std::string& name) const {
	return getTexture(name) != nullptr;
}