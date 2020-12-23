#include"TextureMgr.h"
#include"Util.h"


std::weak_ptr<Texture> TextureManager::addTexture(const std::string& fileName, std::string name) {
	if (name.empty())
		name = ExtractFileNameFromPath(fileName);

	auto found = getTexture(name);
	if (!found.expired())
		return found;
	
	auto imagePath = getResourcePath(fileName);
	if (!imagePath.first)
		return std::weak_ptr<Texture>();

	auto texture = std::make_shared<Texture>();
	if (!texture->loadImage2DFromFile(imagePath.second))
		return std::weak_ptr<Texture>();

	m_textures.insert(std::make_pair(name, texture));
	
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

std::pair<bool, std::string> TextureManager::getResourcePath(const std::string& fileName) const {
	auto res = FileSystem::Default.getHomeDirectory();
	res /= "res/images";
	res /= fileName;

	if (!res.has_extension()) {
		std::cerr << "Walling: file \"" << res.string() << "\" without extendsion!" << std::endl;
#ifdef _DEBUG
		ASSERT(false);
#endif // DEBUG
		return std::make_pair<bool, std::string>(false, "");
	}

	if (!fs::exists(res)) {
		std::cerr << "Walling: file \"" << res.string() << "\" not exist!" << std::endl;
#ifdef _DEBUG
		ASSERT(false);
#endif // DEBUG;
		return std::make_pair<bool, std::string>(false, "");
	}

	res = fs::canonical(res);

	return std::make_pair<bool, std::string>(true, res.string());
}