#pragma once
#include<glad/glad.h>
#include<string>
#include"RendererCore.h"


class Texture {
	friend class SkyboxComponent;

public:
	enum class Unit {
		Defualt,
		DiffuseMap,
		NormalMap,
		SpecularMap,
		EmissiveMap,
		CubeMap,
		ShadowMap,
		Position,
		MaxUnit,
	};

	enum class Target {  // texture unit bindable target
		Unknown,
		Texture_1D = GL_TEXTURE_1D,
		Texture_1D_Array = GL_TEXTURE_1D_ARRAY,
		Texture_2D = GL_TEXTURE_2D,
		Texture_2D_Array = GL_TEXTURE_2D_ARRAY,
		Texture_2D_MultiSample = GL_TEXTURE_2D_MULTISAMPLE,
		Texture_2D_MultiSample_Array = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
		Texture_3D = GL_TEXTURE_3D,
		Texture_CubeMap = GL_TEXTURE_CUBE_MAP,
		Texture_CubeMap_Array = GL_TEXTURE_CUBE_MAP_ARRAY,
		Texture_Rectangle = GL_TEXTURE_RECTANGLE,
		Texture_Buffer = GL_TEXTURE_BUFFER,
	};

	enum class Format {
		Unknown,
		// unsized format
		Depth = GL_DEPTH_COMPONENT,				
		Depth_Stencil = GL_DEPTH_STENCIL,
		R = GL_RED,
		RG = GL_RG,
		RGB = GL_RGB,
		RGBA = GL_RGBA,
		Luminance_Alpha = GL_LUMINANCE_ALPHA,
		Luminance = GL_LUMINANCE,
		Alpha = GL_ALPHA,

		//normalize sized format (none filterable)
		// fragment shader fetch as [0, 1], [-1, 1]
		R8 = GL_R8,
		R8_SNORM = GL_R8_SNORM,
		RG8 = GL_RG8,
		RG8_SNORM = GL_RG8_SNORM,
		RGB8 = GL_RGB8,
		RGB8_SNORM = GL_RGB8_SNORM,
		RGBA8 = GL_RGBA8,
		RGBA8_SNORM = GL_RGBA8_SNORM,
		RGB10A2 = GL_RGB10_A2,

		// sized depth format
		Depth16 = GL_DEPTH_COMPONENT16,
		Depth24 = GL_DEPTH_COMPONENT24,
		Depth32 = GL_DEPTH_COMPONENT32,
		Depth32F = GL_DEPTH_COMPONENT32F,
		Depth24_Stencil8 = GL_DEPTH24_STENCIL8,
		Depth32F_Stencil8 = GL_DEPTH32F_STENCIL8,

		// sized srgb
		SRGB8 = GL_SRGB8,
		SRGBA8 = GL_SRGB8_ALPHA8,

		// float format
		R16F = GL_R16F,
		R32F = GL_R32F,
		RG16F = GL_RG16F,
		RG32F = GL_RG32F,
		RGB16F = GL_RGB16F,
		RGB32F = GL_RGB32F,
		RGBA16F = GL_RGBA16F,
		RGBA32F = GL_RGBA32F,
		R11G11B10F = GL_R11F_G11F_B10F,


		// integer format 
		// fragment shader fetch as integer
		R8I = GL_R8I,
		R8UI = GL_R8UI,
		RG8I = GL_RGB8I,
		RG8UI = GL_RG8UI,
		RGB8I = GL_RGB8I,
		RGB8UI = GL_RGB8UI,
		RGBA8I = GL_RGBA8I,
		RGBA8UI = GL_RGBA8UI,

		R16I = GL_R16I,
		R16UI = GL_R16UI,
		RG16I = GL_RGB16I,
		RG16UI = GL_RG16UI,
		RGB16I = GL_RGB16I,
		RGB16UI = GL_RGB16UI,
		RGBA16I = GL_RGBA16I,
		RGBA16UI = GL_RGBA16UI,

		R32I = GL_R32I,
		R32UI = GL_R32UI,
		RG32I = GL_RGB32I,
		RG32UI = GL_RG32UI,
		RGB32I = GL_RGB32I,
		RGB32UI = GL_RGB32UI,
		RGBA32I = GL_RGBA32I,
		RGBA32UI = GL_RGBA32UI,
	};

	enum class FormatDataType {
		Unknown,
		Byte = GL_BYTE,
		UByte = GL_UNSIGNED_BYTE,
		Short = GL_SHORT,
		UShort = GL_UNSIGNED_SHORT,
		Int = GL_INT,
		UInt = GL_UNSIGNED_INT,
		HFloat = GL_HALF_FLOAT,
		Float = GL_FLOAT,
		UInt_24_8 = GL_UNSIGNED_INT_24_8, // depth_ stencil
	};

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
	Texture();
	virtual ~Texture();

	Texture(const Texture& other) = delete;
	Texture& operator = (const Texture& other) = delete;

	static int maximumAvaliableTextureUnit();

	// load functions can accept unsized/sized internal(gpu) format, unsized external(cpu) format
	bool loadImage2DFromFile(const std::string& file, bool flipUV = true, bool genMipMap = true);
	bool loadImage2DFromMemory(Format gpuFmt, Format cpuFmt, FormatDataType fmtDataType, size_t width, size_t height, const void* data, bool genMipMap = false);
	bool loadCubeMapFromFiles(const std::string& right, const std::string& left, const std::string& top, const std::string& bottom, const std::string& front, const std::string& back, bool flipUV = false, bool genMipMap = false);	
	bool loadCubeMapFromMemory(Format gpuFmt, Format cpuFmt, FormatDataType cpuFmtDataType, float w, float h, const void* right = nullptr, const void* left = nullptr, const void* top = nullptr, const void* bottom = nullptr, const void* front = nullptr, const void* back = nullptr, bool genMipMap = false);

	// all alloc storage functions must provided sized internal format
	void allocStorage2D(Format fmt, size_t width, size_t height, size_t numlevels = 1);
	void allocStorage2DArray(Format fmt, size_t layers, size_t width, size_t height, size_t numlevels = 1);
	void allocStorageCube(Format fmt, size_t width, size_t height, size_t numlevels = 1);
	void allocStorageCubeArray(Format fmt, size_t layers, size_t width, size_t height, size_t numlevels = 1);
	void allocStorage3D(Format fmt, size_t depth, size_t width, size_t height, size_t numlevels = 0);

	bool bind(Unit unit = Unit::Defualt, Target target = Target::Texture_2D) const ;
	void unbind() const;
	void release();

	void setFilterMode(FilterType type, FilterMode mode);
	void setWrapMode(WrapType type, WrapMode mode);
	void setBorderColor(glm::vec4 color);

	void getPiexls(int level, Texture::Format piexlFmt, Texture::FormatDataType fmtDataType, size_t bufferSz, void* piexls);

	inline size_t getWidth() const {
		return m_width;
	}

	inline size_t getHeight() const {
		return m_height;
	}

	inline std::string getSourcePath() const {
		return m_file;
	}

	inline Format getFormat() const {
		return m_format;
	}


	inline size_t getLayerCount() const {
		return m_numLayer;
	}

	inline size_t getDepth() const {
		return m_depth;
	}

	inline int getHandler() const {
		return m_handler;
	}

protected:
	Format getGenericFormat(size_t channelCnt);

private:
	std::string m_file;
	int m_width;
	int m_height;
	int m_numLayer;
	int m_depth;
	Format m_format;

	mutable Unit m_bindedUnit;
	mutable Target m_bindedTarget;
	GLuint m_handler;
};