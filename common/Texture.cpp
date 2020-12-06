#include"Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include<stb_image/stb_image.h>
#include"Util.h"


Texture::Texture():m_handler(0)
, m_file("")
, m_width(0)
, m_height(0)
, m_numLayer(0)
, m_format(Format::Unknown)
, m_cpuFormat(Format::Unknown)
, m_cpuFormatDataType(FormatDataType::Unknown)
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
		loadImage2DFromMemory(fmt, fmt, FormatDataType::UByte, m_width, m_height, data, genMipMap);

		stbi_image_free(data);
		m_file = file;
		m_numLayer = 1;

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


bool Texture::loadImage2DFromMemory(Format internalFmt, Format srcFmt, 
									FormatDataType srcFmtDataType, 
									size_t width, 
									size_t height,
									const void* data,
									bool genMipMap) {
	if (m_bindedTarget == Target::Unknown) {
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

	if (genMipMap)
		GLCALL(glGenerateTextureMipmap(m_handler));

	m_format = internalFmt;
	m_cpuFormat = srcFmt;
	m_cpuFormatDataType = srcFmtDataType;
	m_width = width;
	m_height = height;
	m_numLayer = 1;

	return true;
}


bool Texture::loadCubeMapFromFiles(const std::string& right,
									const std::string& left,
									const std::string& top,
									const std::string& bottom,
									const std::string& front,
									const std::string& back,
									bool genMipMap) {
	if (m_bindedTarget != Target::Texture_CubeMap)
		return false;

	std::string faces[] = { right, left, top, bottom, front, back };
	std::vector<void*> datas;
	int channelCnt = 0;
	bool ok = true;
	for (size_t i = 0; i < 6; i++) {
		stbi_set_flip_vertically_on_load(false);
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
	if (m_bindedTarget != Target::Texture_CubeMap)
		return false;

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
	m_cpuFormat = cpuFmt;
	m_cpuFormatDataType = cpuFmtDataType;
	m_width = w;
	m_height = h;
	m_numLayer = 6;

	setFilterMode(FilterType::Magnification, FilterMode::Liner);
	setFilterMode(FilterType::Minification, FilterMode::Liner);
	setWrapMode(WrapType::S, WrapMode::Clamp_To_Edge);
	setWrapMode(WrapType::T, WrapMode::Clamp_To_Edge);
	setWrapMode(WrapType::R, WrapMode::Clamp_To_Edge);

	return true;
}


bool Texture::loadImage2DArrayFromMemory(Format gpuFmt,
										Format cpuFmt,
										FormatDataType cpuFmtDataType,
										size_t width,
										size_t height,
										size_t numLayer,
										const void* data,
										bool genMipMap) {
	if (m_bindedTarget != Target::Texture_2D_Array)
		return false;

	GLCALL(glTexImage3D(GL_TEXTURE_2D_ARRAY,
		0,
		int(gpuFmt),
		width,
		height,
		numLayer,
		0,
		int(cpuFmt),
		int(cpuFmtDataType),
		data));
	
	if (genMipMap)
		GLCALL(glGenerateTextureMipmap(m_handler));

	setFilterMode(FilterType::Minification, FilterMode::Liner);
	setFilterMode(FilterType::Magnification, FilterMode::Liner);
	setWrapMode(WrapType::S, WrapMode::Clamp_To_Edge);
	setWrapMode(WrapType::T, WrapMode::Clamp_To_Edge);

	m_format = gpuFmt;
	m_cpuFormat = cpuFmt;
	m_cpuFormatDataType = cpuFmtDataType;
	m_width = width;
	m_height = height;
	m_numLayer = numLayer;

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
		glDeleteTextures(1, &m_handler);
		m_handler = 0;
		m_width = 0;
		m_height = 0;
		m_numLayer = 0;
		m_file.clear();
		m_format = Format::Unknown;
		m_cpuFormat = Format::Unknown;
		m_cpuFormatDataType = FormatDataType::Unknown;
		m_bindedUnit = Unit::Defualt;
		m_bindedTarget = Target::Unknown;
	}
}