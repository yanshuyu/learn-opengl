#include"Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include<stb_image/stb_image.h>
#include"Util.h"


Texture::Texture():m_handler(0)
, m_file("")
, m_width(0)
, m_height(0)
, m_numLayer(0)
, m_depth(0)
, m_format(Format::Unknown)
, m_bindedUnit(Unit::Defualt)
, m_bindedTarget(Target::Unknown)
, m_bindAsImage(false)
, m_bindedImageUnit(-1) {
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


bool Texture::loadImage2DFromFile(const std::string& file, bool flipUV, bool genMipMap) {
	int channelCnt = 0;
	stbi_set_flip_vertically_on_load(flipUV);
	auto data = stbi_load(file.c_str(), &m_width, &m_height, &channelCnt, 0);

	if (data) {
		auto fmt = getGenericFormat(channelCnt);
		loadImage2DFromMemory(fmt, fmt, FormatDataType::UByte, m_width, m_height, data, genMipMap);
		stbi_image_free(data);
		m_file = file;

		return true;
	}

#ifdef _DEBUG
	std::string msg;
	msg += "[Texture Load error] Failed to load texture: ";
	msg += m_file;
	CONSOLELOG(msg);
#endif // _DEBUG

	return false;
}


bool Texture::loadImage2DFromMemory(Format cpuFmt, Format gpuFmt, 
									FormatDataType mtDataType, 
									size_t width, 
									size_t height,
									const void* data,
									bool genMipMap) {
	bind();
	GLCALL(glTexImage2D(GL_TEXTURE_2D,
		0,
		GLint(gpuFmt),
		width,
		height,
		0,
		GLenum(cpuFmt),
		int(mtDataType),
		data));

	setFilterMode(FilterType::Magnification, FilterMode::Liner);
	setFilterMode(FilterType::Minification, FilterMode::Liner);
	setWrapMode(WrapType::S, WrapMode::Clamp_To_Edge);
	setWrapMode(WrapType::T, WrapMode::Clamp_To_Edge);
	setWrapMode(WrapType::R, WrapMode::Clamp_To_Edge);

	if (genMipMap)
		GLCALL(glGenerateTextureMipmap(m_handler));

	m_format = gpuFmt;
	m_width = width;
	m_height = height;
	m_numLayer = 1;
	m_depth = 1;

	unbind();

	return true;
}


bool Texture::loadCubeMapFromFiles(const std::string& right,
									const std::string& left,
									const std::string& top,
									const std::string& bottom,
									const std::string& front,
									const std::string& back,
									bool flipUV,
									bool genMipMap) {
	std::string faces[] = { right, left, top, bottom, front, back };
	std::vector<void*> datas;
	int channelCnt = 0;
	bool ok = true;
	for (size_t i = 0; i < 6; i++) {
		stbi_set_flip_vertically_on_load(flipUV);
		auto data = stbi_load(faces[i].c_str(), &m_width, &m_height, &channelCnt, 0);
		if (!data) {
			ok = false;
			break;
		}

		datas.push_back(data);
	}

	if (!ok) {
		for (auto data : datas) {
			stbi_image_free(data);
		}
		return false;
	}

	Format fmt = getGenericFormat(channelCnt);
	if (!loadCubeMapFromMemory(fmt, fmt, FormatDataType::UByte, m_width, m_height, datas[0], datas[1], datas[2], datas[3], datas[4], datas[5], genMipMap))
		return false;

	return true;
}

bool Texture::loadCubeMapFromMemory(Format gpuFmt,
									Format cpuFmt,
									FormatDataType cpuFmtDataType,
									float w,
									float h,
									const void* right,
									const void* left,
									const void* top,
									const void* bottom,
									const void* front,
									const void* back,
									bool genMipMap) {
	bind(Unit::Defualt, Target::Texture_CubeMap);

	const void* datas[] = {right, left, top, bottom, front, back };
	for (size_t i = 0; i < 6; i++) {
		if (!datas[i])
			continue;

		GLCALL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			int(gpuFmt),
			w,
			h,
			0,
			int(cpuFmt),
			int(cpuFmtDataType),
			datas[i]));
	}
	
	if (genMipMap)
		GLCALL(glGenerateTextureMipmap(m_handler));

	m_format = gpuFmt;
	m_numLayer = 1;
	m_depth = 1;
	m_width = w;
	m_height = h;

	setFilterMode(FilterType::Magnification, FilterMode::Liner);
	setFilterMode(FilterType::Minification, FilterMode::Liner);
	setWrapMode(WrapType::S, WrapMode::Clamp_To_Edge);
	setWrapMode(WrapType::T, WrapMode::Clamp_To_Edge);
	setWrapMode(WrapType::R, WrapMode::Clamp_To_Edge);

	unbind();

	return true;
}


void Texture::allocStorage2D(Format fmt, size_t width, size_t height, size_t levels) {
	bind();
	GLCALL(glTexStorage2D(GL_TEXTURE_2D, levels, GLenum(fmt), width, height));
	unbind();
	m_format = fmt;
	m_width = width;
	m_height = height;
	m_numLayer = 1;
	m_depth = 1;
}


void Texture::allocStorage2DArray(Format fmt, size_t layers, size_t width, size_t height, size_t levels) {
	bind(Unit::Defualt, Target::Texture_2D_Array);
	GLCALL(glTexStorage3D(GL_TEXTURE_2D_ARRAY, levels, GLenum(fmt), width, height, layers));
	unbind();
	m_format = fmt;
	m_width = width;
	m_height = height;
	m_numLayer = layers;
	m_depth = 1;
}


void Texture::allocStorageCube(Format fmt, size_t width, size_t height, size_t levels) {
	bind(Unit::Defualt, Target::Texture_CubeMap);
	GLCALL(glTexStorage2D(GL_TEXTURE_CUBE_MAP, levels, GLenum(fmt), width, height));
	unbind();
	m_format = fmt;
	m_numLayer = 1;
	m_depth = 1;
	m_width = width;
	m_height = height;

}


void Texture::allocStorageCubeArray(Format fmt, size_t layers, size_t width, size_t height, size_t levels) {
	bind(Unit::Defualt, Target::Texture_CubeMap_Array);
	GLCALL(glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, levels, GLenum(fmt), width, height, layers));
	unbind();
	m_format = fmt;
	m_numLayer = layers;
	m_depth = 1;
	m_width = width;
	m_height = height;
}


void Texture::allocStorage3D(Format fmt, size_t depth, size_t width, size_t height, size_t levels) {
	bind(Unit::Defualt, Target::Texture_3D);
	GLCALL(glTexStorage3D(GL_TEXTURE_3D, levels, GLenum(fmt), width, height, depth));
	unbind();
	m_format = fmt;
	m_numLayer = 1;
	m_depth = depth;
	m_width = width;
	m_height = height;
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
		m_bindedTarget = Target::Unknown;
		m_bindedUnit = Unit::Defualt;
	}
}


void Texture::bindToImageUnit(size_t unit, Format fmt, Access access, size_t level, bool bindAsArray, size_t layerToBind) {
	GLCALL(glBindImageTexture(unit, m_handler, level, bindAsArray, layerToBind, GLenum(access), GLenum(fmt)));
	m_bindAsImage = true;
	m_bindedImageUnit = unit;
}


void Texture::unbindFromImageUnit() const {
	if (m_bindAsImage) {
		GLCALL(glBindImageTexture(m_bindedImageUnit, m_handler, 0, false, 0, GLenum(Access::Read), GLenum(m_format)));
		m_bindAsImage = false;
		m_bindedImageUnit = -1;
	}
}


void Texture::setFilterMode(FilterType type, FilterMode mode) {
	GLCALL(glTextureParameteri(m_handler,  GLenum(type), GLenum(mode)));
}

void Texture::setWrapMode(WrapType type, WrapMode mode) {
	GLCALL(glTextureParameteri(m_handler, GLenum(type), GLenum(mode)));
}

void Texture::setBorderColor(glm::vec4 color) {
	GLCALL(glTextureParameterfv(m_handler, GL_TEXTURE_BORDER_COLOR, &color[0]));
}


void Texture::getPiexls(int level, Texture::Format piexlFmt, Texture::FormatDataType fmtDataType, size_t bufferSz, void* piexls) {
	if (!piexls)
		return;

	GLCALL(glGetTextureImage(m_handler, level, int(piexlFmt), int(fmtDataType), bufferSz, piexls));
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
		unbind();
		unbindFromImageUnit();
		glDeleteTextures(1, &m_handler);
		m_handler = 0;
		m_width = 0;
		m_height = 0;
		m_numLayer = 0;
		m_depth = 0;
		m_file.clear();
		m_format = Format::Unknown;
		m_bindedUnit = Unit::Defualt;
		m_bindedTarget = Target::Unknown;
	}
}