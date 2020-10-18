#include"TextureMgr.h"
#include"Util.h"


std::weak_ptr<Texture> TextureManager::addTexture(const std::string& file, const std::string& name) {
	std::string textureName(name);
	if (textureName.empty())
		textureName = ExtractFileNameFromPath(file);
	if (textureName.empty())
		textureName = file;

	auto found = getTexture(textureName);
	if (!found.expired())
		return found;
	
	auto texture = std::make_shared<Texture>();
	if (!texture->loadImage2DFromFile(file))
		return std::weak_ptr<Texture>();

	m_textures.insert(std::make_pair(textureName, texture));
	
	return std::weak_ptr<Texture>(texture);
}

std::weak_ptr<Texture> TextureManager::getTexture(const std::string& name) const {
	auto pos = m_textures.find(name);
	if (pos != m_textures.end())
		return std::weak_ptr<Texture>(pos->second);
	
	return std::weak_ptr<Texture>();
}


bool TextureManager::removeTexture(const std::string& name) {
	auto pos = m_textures.find(name);
	if (pos != m_textures.end()) {
		m_textures.erase(pos);
		return true;
	}

	return false;
}


bool TextureManager::hasTexture(const std::string& name) const {
	return !getTexture(name).expired();
}