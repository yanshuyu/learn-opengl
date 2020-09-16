#include"Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include<stb_image/stb_image.h>
#include"Util.h"


Texture::Texture(const std::string& file) :m_file(file)
, m_handler(0)
, m_loaded(false)
, m_width(0)
, m_height(0)
, m_channelCount(0) {

}


Texture::~Texture() {
	release();
}

bool Texture::load() {
	if (m_loaded)
		return true;
	stbi_set_flip_vertically_on_load(true);
	auto data = stbi_load(m_file.c_str(), &m_width, &m_height, &m_channelCount, 0);

	if (data) {
		GLCALL(glGenTextures(1, &m_handler));
		GLCALL(glBindTexture(GL_TEXTURE_2D, m_handler));
		GLCALL(glTexImage2D(GL_TEXTURE_2D,
								0,
								GL_RGBA8,
								m_width,
								m_height,
								0,
								formatFromChannelCount(),
								GL_UNSIGNED_BYTE,
								data));
		setFilterMode(FilterType::Magnification, FilterMode::Liner);
		setFilterMode(FilterType::Minification, FilterMode::Liner);
		setWrapMode(WrapType::S, WrapMode::Clamp_To_Edge);
		setWrapMode(WrapType::T, WrapMode::Clamp_To_Edge);

		m_loaded = true;
	}

	if (!m_loaded) {
#ifdef _DEBUG
		std::string msg;
		msg += "[Texture Load error] Failed to load texture: ";
		msg += m_file;
		CONSOLELOG(msg);
#endif // _DEBUG

	}

	stbi_image_free(data);
	
	return m_loaded;
}

void Texture::bind(int slot) const {
	GLCALL(glActiveTexture(GL_TEXTURE0 + slot));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_handler));
}

void Texture::unbind() const {
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}


void Texture::setFilterMode(FilterType type, FilterMode mode) {
	GLCALL(glTextureParameteri(m_handler,  GLenum(type), GLenum(mode)));
}

void Texture::setWrapMode(WrapType type, WrapMode mode) {
	GLCALL(glTextureParameteri(m_handler, GLenum(type), GLenum(mode)));
}

GLenum Texture::formatFromChannelCount() {
	switch (m_channelCount)
	{
	case 1:
		return GL_LUMINANCE;
	case 2:
		return GL_LUMINANCE_ALPHA;
	case 3:
		return GL_RGB;
	case 4:
		return GL_RGBA;

	default:
		ASSERT(false);
	}
}


void Texture::release() {
	if (m_handler) {
		glDeleteTextures(1, &m_handler);
		m_handler = 0;
		m_loaded = false;
		m_width = 0;
		m_height = 0;
		m_channelCount = 0;
	}
}