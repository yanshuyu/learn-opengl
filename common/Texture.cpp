#include"Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include<stb_image/stb_image.h>
#include"Util.h"


Texture::Texture():m_handler(0)
, m_file("")
, m_width(0)
, m_height(0)
, m_format(Format::Unknown)
, m_bindedUnit(Unit::Defualt)
, m_bindedTarget(Target::Unknown) {
	GLCALL(glGenTextures(1, &m_handler));
}


Texture::~Texture() {
	release();
}


int Texture::maximumAvaliableTextureUnit() {
	int maxUnit = 0;
	GLCALL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnit));
	return maxUnit;
}


bool Texture::loadImage2DFromFile(const std::string& file, bool genMipMap) {
	bind();
	int channelCnt = 0;
	stbi_set_flip_vertically_on_load(true);
	auto data = stbi_load(file.c_str(), &m_width, &m_height, &channelCnt, 0);

	if (data) {
		auto fmt = getGenericFormat(channelCnt);
		loadImage2DFromMemory(fmt, fmt, FormatDataType::UByte, m_width, m_height, data);

		if (genMipMap)
			GLCALL(glGenerateTextureMipmap(m_handler));

		stbi_image_free(data);
		m_file = file;

		unbind();

		return true;
	}

#ifdef _DEBUG
	std::string msg;
	msg += "[Texture Load error] Failed to load texture: ";
	msg += m_file;
	CONSOLELOG(msg);
#endif // _DEBUG

	unbind();
	
	return false;
}


bool Texture::loadImage2DFromMemory(Format internalFmt, Format srcFmt, FormatDataType srcFmtDataType, size_t width, size_t height, const void* data) {
	if (m_bindedTarget == Target::Unknown) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return false;
	}

	GLCALL(glTexImage2D(int(m_bindedTarget),
		0,
		GLint(internalFmt),
		width,
		height,
		0,
		GLenum(srcFmt),
		int(srcFmtDataType),
		data));

	setFilterMode(FilterType::Magnification, FilterMode::Liner);
	setFilterMode(FilterType::Minification, FilterMode::Liner);
	setWrapMode(WrapType::S, WrapMode::Clamp_To_Edge);
	setWrapMode(WrapType::T, WrapMode::Clamp_To_Edge);

	m_format = internalFmt;
	m_width = width;
	m_height = height;

	return true;
}


bool Texture::loadCubeMapFromFiles(const std::string& left,
									const std::string& right,
									const std::string& top,
									const std::string& bottom,
									const std::string& front,
									const std::string& back) {
	std::string faces[] = { right, left, top, bottom, front, back };
	bind(Unit::Defualt, Target::Texture_CubeMap);
	
	for (size_t i = 0; i < 6; i++) {
		int channelCnt = 0;
		stbi_set_flip_vertically_on_load(true);
		auto data = stbi_load(faces[i].c_str(), &m_width, &m_height, &channelCnt, 0);
		
		if (data) {
			m_format = getGenericFormat(channelCnt); 
			GLCALL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				int(m_format),
				m_width,
				m_height,
				0,
				int(m_format),
				GL_UNSIGNED_BYTE,
				data));
		} else {
			unbind();
			return false;
		}

		stbi_image_free(data);
	}

	setFilterMode(FilterType::Magnification, FilterMode::Liner);
	setFilterMode(FilterType::Minification, FilterMode::Liner);
	setWrapMode(WrapType::S, WrapMode::Clamp_To_Edge);
	setWrapMode(WrapType::T, WrapMode::Clamp_To_Edge);

	unbind();

	return true;
}


bool Texture::bind(Unit unit, Target target)  const {
	if (int(unit) >= maximumAvaliableTextureUnit()) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return false;
	}

	GLCALL(glActiveTexture(GL_TEXTURE0 + int(unit)));
	GLCALL(glBindTexture(int(target), m_handler));
	m_bindedUnit = unit;
	m_bindedTarget = target;
	return true;
}

void Texture::unbind() const {
	if (m_bindedTarget != Target::Unknown) {
		GLCALL(glActiveTexture(GL_TEXTURE0 + int(m_bindedUnit)));
		GLCALL(glBindTexture(int(m_bindedTarget), 0));
		m_bindedUnit = Unit::Defualt;
		m_bindedTarget = Target::Unknown;
	}
}


void Texture::setFilterMode(FilterType type, FilterMode mode) {
	GLCALL(glTextureParameteri(m_handler,  GLenum(type), GLenum(mode)));
}

void Texture::setWrapMode(WrapType type, WrapMode mode) {
	GLCALL(glTextureParameteri(m_handler, GLenum(type), GLenum(mode)));
}

Texture::Format Texture::getGenericFormat(size_t channelCnt) {
	switch (channelCnt)
	{
	case 1:
		return Format::Luminance;
	case 2:
		return Format::Luminance_Alpha;
	case 3:
		return Format::RGB;
	case 4:
		return Format::RGBA;
	default:
#ifdef _DEBUG
		ASSERT(false);
#endif // 
		return Format::Unknown;
	}
}


void Texture::release() {
	if (m_handler) {
		glDeleteTextures(1, &m_handler);
		m_handler = 0;
		m_width = 0;
		m_height = 0;
		m_file.clear();
		m_format = Format::Unknown;
		m_bindedUnit = Unit::Defualt;
		m_bindedTarget = Target::Unknown;
	}
}