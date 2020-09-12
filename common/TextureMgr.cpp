#include"TextureMgr.h"
#include"Util.h"


std::shared_ptr<Texture> TextureManager::addTexture(const std::string& file, const std::string& name) {
	std::string textureName(name);
	if (textureName.empty())
		if (!ExtractFileNameFromPath(file, textureName))
			textureName = file;

	auto texture = getTexture(textureName);
	if (texture != nullptr)
		return texture;
	
	texture = std::make_shared<Texture>(file);
	
	if (!texture->load())
		return nullptr;

	m_textures.insert(std::make_pair(textureName, texture));
	
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