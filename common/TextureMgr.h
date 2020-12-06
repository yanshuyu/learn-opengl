#pragma once
#include"Singleton.h"
#include"Texture.h"
#include"FileSystem.h"
#include<unordered_map>
#include<string>
#include<memory>


class TextureManager : public Singleton<TextureManager> {
public:

	std::weak_ptr<Texture> addTexture(const std::string& fileName, std::string name = "");
	std::weak_ptr<Texture> getTexture(const std::string& name) const;
	bool removeTexture(const std::string& name);
	bool hasTexture(const std::string& name) const;
	
	inline void removeAllTextures() {
		m_textures.clear();
	}

	inline size_t textureCount() const {
		return m_textures.size();
	}


	std::string getResourcePath(const std::string& fileName) const;

private:
	std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
};

