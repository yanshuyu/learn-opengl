#pragma once
#include"Singleton.h"
#include"Texture.h"
#include<unordered_map>
#include<string>
#include<memory>


class TextureManager : public Singleton<TextureManager> {
public:

	std::shared_ptr<Texture> addTexture(const std::string& file, const std::string& name = "");
	std::shared_ptr<Texture> getTexture(const std::string& name) const;
	std::shared_ptr<Texture> removeTexture(const std::string& name);
	bool hasTexture(const std::string& name) const;
	
	inline void removeAllTextures() {
		m_textures.clear();
	}

	inline size_t textureCount() const {
		return m_textures.size();
	}

	inline std::string getResourceAbsolutePath() const {
		return "C:/Users/SY/Documents/learn-opengl/res/images/";
	}

	inline std::string getResourceRelativePath() const {
		return "res/images/";
	}

private:
	std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
};

