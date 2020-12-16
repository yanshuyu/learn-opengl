#pragma once
#include<glad/glad.h>



class Buffer {
public:
	enum class Target {
		Unknown,
		VertexBuffer = GL_ARRAY_BUFFER,
		IndexBuffer = GL_ELEMENT_ARRAY_BUFFER,
		UniformBuffer = GL_UNIFORM_BUFFER,
		TransformFeedBackBuffer = GL_TRANSFORM_FEEDBACK_BUFFER,
		CopyReadBuffer = GL_COPY_READ_BUFFER,
		CopyWriteBuffer = GL_COPY_WRITE_BUFFER,
		ShaderStorageBuffer = GL_SHADER_STORAGE_BUFFER,
		TextureBuffer = GL_TEXTURE_BUFFER,
		PixelPackBuffer = GL_PIXEL_PACK_BUFFER,
		PixelUnpackBuffer = GL_PIXEL_UNPACK_BUFFER,
		AtomicCounterBuffer = GL_ATOMIC_COUNTER_BUFFER,
	};

	enum class Usage {
		Unknown,
		StaticDraw = GL_STATIC_DRAW,
		DynamicDraw = GL_DYNAMIC_DRAW,
		StaticRead = GL_STATIC_READ,
		DynamicRead = GL_DYNAMIC_READ,
		StaticCopy = GL_STATIC_COPY,
		DynamicCopy = GL_DYNAMIC_COPY,
		StreamDraw = GL_STREAM_DRAW,
		StreamRead = GL_STREAM_READ,
		StreamCopy = GL_STREAM_COPY,

	};

	enum class MapAccess {
		Read = GL_READ_ONLY,
		Write = GL_WRITE_ONLY,
		ReadWrite = GL_READ_WRITE,
	};

	enum class MapBitFiled {
		Read_Bit = GL_MAP_READ_BIT,
		Write_Bt = GL_MAP_WRITE_BIT,
		Resitent_Bit = GL_MAP_PERSISTENT_BIT,
		Coherent_Bit = GL_MAP_COHERENT_BIT,
		Invalidate_Range_Bit = GL_MAP_INVALIDATE_RANGE_BIT,
		Invalidate_Buffer_Bit = GL_MAP_INVALIDATE_BUFFER_BIT,
		Flush_Explicit_Bit = GL_MAP_FLUSH_EXPLICIT_BIT,
		Unsynchronized_Bit = GL_MAP_UNSYNCHRONIZED_BIT,
	};
public:
	Buffer();
	Buffer(const void* data, size_t dataSz, Target target, Usage usage, size_t elementCnt = 0);
	~Buffer();

	Buffer(const Buffer& other) = delete;
	Buffer& operator = (const Buffer& other) = delete;

	void bind(Target target) const;
	void bindBase(Target target, size_t index) const; // bind to indexed buffer target
	void bindRange(Target target, size_t index, size_t dataOffset, size_t dataSz) const; // bind range data of buffer to indexed buffer target
	void unbind() const;
	void release();

	bool loadData(const void* data, size_t dataSz, Usage usage, size_t elementCnt = 0);
	bool loadSubData(const void* data, size_t dataOffset, size_t dataSz); //updates a subset of a buffer's data store

	void* map(MapAccess access);
	void* mapRange(MapBitFiled mbf, size_t offset, size_t len);
	bool unmap();

	inline GLuint getHandler() const {
		return m_handler;
	}

	inline size_t getSize() const {
		return m_size;
	}

	inline size_t getElementCount() const {
		return m_elementCount;
	}

	inline Target getTarget() const {
		return m_target;
	}

	inline Usage getUsage() const {
		return m_usage;
	}

protected:
	GLuint m_handler;
	size_t m_size; //buffer size in bytes
	size_t m_elementCount; //element count in buffer
	mutable Target m_target;
	Usage m_usage;

};