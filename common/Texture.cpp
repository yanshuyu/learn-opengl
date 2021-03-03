#include"Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include<stb_image/stb_image.h>
#include"Util.h"
#include"Buffer.h"


Texture::Texture():m_handler(0)
, m_file("")
, m_width(0)
, m_height(0)
, m_numLayer(0)
, m_depth(0)
, m_format(Format::Unknown)
, m_bindedTexUnit(Unit::Defualt)
, m_bindedTarget(Target::Unknown)
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
	bindToTextureUnit();
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

	if (genMipMap) {
		//GLCALL(glGenerateTextureMipmap(m_handler));
		GLCALL(glGenerateMipmap(GLenum(m_bindedTarget)));
	}

	m_format = gpuFmt;
	m_width = width;
	m_height = height;
	m_numLayer = 1;
	m_depth = 1;

	unbindFromTextureUnit();

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
	bindToTextureUnit(Unit::Defualt, Target::Texture_CubeMap);

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
	
	if (genMipMap) {
		//GLCALL(glGenerateTextureMipmap(m_handler));
		GLCALL(glGenerateMipmap(GLenum(m_bindedTarget)));
	}

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

	unbindFromTextureUnit();

	return true;
}


void Texture::allocStorage2D(Format fmt, size_t width, size_t height, size_t levels) {
	bindToTextureUnit();
	GLCALL(glTexStorage2D(GL_TEXTURE_2D, levels, GLenum(fmt), width, height));
	unbindFromTextureUnit();
	m_format = fmt;
	m_width = width;
	m_height = height;
	m_numLayer = 1;
	m_depth = 1;
}


void Texture::allocStorage2DArray(Format fmt, size_t layers, size_t width, size_t height, size_t levels) {
	bindToTextureUnit(Unit::Defualt, Target::Texture_2D_Array);
	GLCALL(glTexStorage3D(GL_TEXTURE_2D_ARRAY, levels, GLenum(fmt), width, height, layers));
	unbindFromTextureUnit();
	m_format = fmt;
	m_width = width;
	m_height = height;
	m_numLayer = layers;
	m_depth = 1;
}


void Texture::allocStorageCube(Format fmt, size_t width, size_t height, size_t levels) {
	bindToTextureUnit(Unit::Defualt, Target::Texture_CubeMap);
	GLCALL(glTexStorage2D(GL_TEXTURE_CUBE_MAP, levels, GLenum(fmt), width, height));
	unbindFromTextureUnit();
	m_format = fmt;
	m_numLayer = 1;
	m_depth = 1;
	m_width = width;
	m_height = height;

}


void Texture::allocStorageCubeArray(Format fmt, size_t layers, size_t width, size_t height, size_t levels) {
	bindToTextureUnit(Unit::Defualt, Target::Texture_CubeMap_Array);
	GLCALL(glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, levels, GLenum(fmt), width, height, layers));
	unbindFromTextureUnit();
	m_format = fmt;
	m_numLayer = layers;
	m_depth = 1;
	m_width = width;
	m_height = height;
}


void Texture::allocStorage3D(Format fmt, size_t depth, size_t width, size_t height, size_t levels) {
	bindToTextureUnit(Unit::Defualt, Target::Texture_3D);
	GLCALL(glTexStorage3D(GL_TEXTURE_3D, levels, GLenum(fmt), width, height, depth));
	unbindFromTextureUnit();
	m_format = fmt;
	m_numLayer = 1;
	m_depth = depth;
	m_width = width;
	m_height = height;
}


bool Texture::subDataImage2D(const void* data, Format fmt, FormatDataType fmtDataType, size_t xOffset, size_t yOffset, size_t w, size_t h, size_t mipLevel) {
	if (m_bindedTarget == Target::Unknown)
		return false;
	//GLCALL(glTextureSubImage2D(m_handler, mipLevel, xOffset, yOffset, w, h, GLenum(fmt), GLenum(fmtDataType), data));
	GLCALL(glTexSubImage2D(GLenum(m_bindedTarget), mipLevel, xOffset, yOffset, w, h, GLenum(fmt), GLenum(fmtDataType), data));
	return true;
}

bool Texture::subDataImage2DFromBuffer(Buffer* buf, Format fmt, FormatDataType fmtDataType, size_t xOffset, size_t yOffset, size_t w, size_t h, size_t mipLevel, size_t byteOffset) {
	if (m_bindedTarget == Target::Unknown)
		return false;

	buf->bind(Buffer::Target::PixelUnpackBuffer);
	//GLCALL(glTextureSubImage2D(m_handler, mipLevel, xOffset, yOffset, w, h, GLenum(fmt), GLenum(fmtDataType), reinterpret_cast<void*>(byteOffset)));
	GLCALL(glTexSubImage2D(GLenum(m_bindedTarget), mipLevel, xOffset, yOffset, w, h, GLenum(fmt), GLenum(fmtDataType), reinterpret_cast<void*>(byteOffset)));
	buf->unbind();
	
	return true;
}

bool Texture::subDataImage2DArray(const void* data, Format fmt, FormatDataType fmtDataType, size_t xOffset, size_t yOffset, size_t w, size_t h, size_t slice, size_t mipLevel) {
	if (m_bindedTarget == Target::Unknown)
		return false;
	//GLCALL(glTextureSubImage3D(m_handler, mipLevel, xOffset, yOffset, slice, w, h, 0, GLenum(fmt), GLenum(fmtDataType), data));
	GLCALL(glTexSubImage3D(GLenum(m_bindedTarget), mipLevel, xOffset, yOffset, slice, w, h, 0, GLenum(fmt), GLenum(fmtDataType), data));
	return true;
}

bool Texture::subDataImage2DArrayFromBuffer(Buffer* buf, Format fmt, FormatDataType fmtDataType, size_t xOffset, size_t yOffset, size_t w, size_t h, size_t slice, size_t mipLevel, size_t byteOffset) {
	if (m_bindedTarget == Target::Unknown)
		return false;

	buf->bind(Buffer::Target::PixelUnpackBuffer);
	//GLCALL(glTextureSubImage3D(m_handler, mipLevel, xOffset, yOffset, slice, w, h, 0, GLenum(fmt), GLenum(fmtDataType), &byteOffset));
	GLCALL(glTexSubImage3D(GLenum(m_bindedTarget), mipLevel, xOffset, yOffset, slice, w, h, 0, GLenum(fmt), GLenum(fmtDataType), &byteOffset));
	buf->unbind();

	return true;
}


bool Texture::bindToTextureUnit(Unit unit, Target target)  const {
	GLCALL(glActiveTexture(GL_TEXTURE0 + int(unit)));
	GLCALL(glBindTexture(int(target), m_handler));
	m_bindedTexUnit = unit;
	m_bindedTarget = target;
	return true;
}


void Texture::unbindFromTextureUnit() const {
	if (m_bindedTarget != Target::Unknown) {
		GLCALL(glActiveTexture(GL_TEXTURE0 + int(m_bindedTexUnit)));
		GLCALL(glBindTexture(int(m_bindedTarget), 0));
		m_bindedTarget = Target::Unknown;
		m_bindedTexUnit = Unit::Defualt;
	}
}


void Texture::bindToImageUnit(size_t unit, Format fmt, Access access, size_t level, bool bindAsArray, size_t layerToBind) {
	GLCALL(glBindImageTexture(unit, m_handler, level, bindAsArray, layerToBind, GLenum(access), GLenum(fmt)));
	m_bindedImageUnit = unit;
}


void Texture::unbindFromImageUnit() const {
	if (m_bindedImageUnit >= 0) {
		GLCALL(glBindImageTexture(m_bindedImageUnit, 0, 0, false, 0, GLenum(Access::Read), GLenum(m_format)));
		m_bindedImageUnit = -1;
	}
}


bool Texture::setFilterMode(FilterType type, FilterMode mode) {
	if (m_bindedTarget == Target::Unknown)
		return false;

	//GLCALL(glTextureParameteri(m_handler,  GLenum(type), GLenum(mode)));
	GLCALL(glTexParameteri(GLenum(m_bindedTarget), GLenum(type), GLenum(mode)));

	return true;
}

bool Texture::setWrapMode(WrapType type, WrapMode mode) {
	if (m_bindedTarget == Target::Unknown)
		return false;

	//GLCALL(glTextureParameteri(m_handler, GLenum(type), GLenum(mode)));
	GLCALL(glTexParameteri(GLenum(m_bindedTarget), GLenum(type), GLenum(mode)));

	return true;
}

bool Texture::setBorderColor(glm::vec4 color) {
	if (m_bindedTarget == Target::Unknown)
		return false;

	//GLCALL(glTextureParameterfv(m_handler, GL_TEXTURE_BORDER_COLOR, &color[0]));
	GLCALL(glTexParameterfv(GLenum(m_bindedTarget), GL_TEXTURE_BORDER_COLOR, &color[0]));

	return true;
}


bool Texture::setCompareMode(CompareMode cm) {
	if (m_bindedTarget == Target::Unknown)
		return false;

	//GLCALL(glTextureParameteri(m_handler, GL_TEXTURE_COMPARE_MODE, GLint(cm)));
	GLCALL(glTexParameteri(GLenum(m_bindedTarget), GL_TEXTURE_COMPARE_MODE, GLint(cm)));

	return true;
}

bool Texture::setCompareFunc(CompareFunc cf) {
	if (m_bindedTarget == Target::Unknown)
		return false;

	//GLCALL(glTextureParameteri(m_handler, GL_TEXTURE_COMPARE_FUNC, GLint(cf)));
	GLCALL(glTexParameteri(GLenum(m_bindedTarget), GL_TEXTURE_COMPARE_FUNC, GLint(cf)));

	return true;
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
		unbindFromTextureUnit();
		unbindFromImageUnit();
		glDeleteTextures(1, &m_handler);
		m_handler = 0;
		m_width = 0;
		m_height = 0;
		m_numLayer = 0;
		m_depth = 0;
		m_file.clear();
		m_format = Format::Unknown;
		m_bindedTexUnit = Unit::Defualt;
		m_bindedTarget = Target::Unknown;
	}
}