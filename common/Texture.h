#pragma once
#include<glad/glad.h>
#include<string>


class Texture {
public:
	enum class FilterType {
		Magnification = GL_TEXTURE_MIN_FILTER,
		Minification = GL_TEXTURE_MAG_FILTER,
	};

	enum class FilterMode {
		Nearest = GL_NEAREST,
		Liner = GL_LINEAR,
		Nearest_MipMap_Nearest = GL_NEAREST_MIPMAP_NEAREST,
		Linear_MipMap_Nearest = GL_LINEAR_MIPMAP_NEAREST,
		Nearest_MipMap_Linear = GL_NEAREST_MIPMAP_LINEAR,
		Linear_MipMap_Linear = GL_LINEAR_MIPMAP_LINEAR,
	};

	enum class WrapType {
		S = GL_TEXTURE_WRAP_S,
		T = GL_TEXTURE_WRAP_T,
		R = GL_TEXTURE_WRAP_R,
	};

	enum class WrapMode {
		Clamp_To_Edge = GL_CLAMP_TO_EDGE,
		Clamp_To_Border = GL_CLAMP_TO_BORDER,
		Mirrored_Repeat = GL_MIRRORED_REPEAT,
		Repeat = GL_REPEAT,
		Mirrored_Clamp_To_Egde = GL_MIRROR_CLAMP_TO_EDGE,
	};
public:
	Texture(const std::string& file);
	virtual ~Texture();

	bool load();
	void bind(int slot = 0) const ;
	void unbind() const;
	void release();

	void setFilterMode(FilterType type, FilterMode mode);
	void setWrapMode(WrapType type, WrapMode mode);

	inline bool isLoaded() const {
		return m_loaded;
	}

	inline size_t getWidth() const {
		return m_width;
	}

	inline size_t getHeight() const {
		return m_height;
	}

	inline size_t getChannelCount() const {
		return m_channelCount;
	}

	inline std::string getPath() const {
		return m_file;
	}

	inline int getHandler() const {
		return m_handler;
	}

protected:
	GLenum formatFromChannelCount();

protected:
	std::string m_file;
	int m_width;
	int m_height;
	int m_channelCount;
	bool m_loaded;
	GLuint m_handler;
};