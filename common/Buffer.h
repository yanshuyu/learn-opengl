#pragma once
#include<glad/glad.h>



class Buffer {
public:
	enum class MapAccess {
		Read_Only = GL_READ_ONLY,
		Write_Only = GL_WRITE_ONLY,
		Read_Write = GL_READ_WRITE,
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
	Buffer(const void* data, size_t sz, GLenum target, GLenum usage, size_t elementCnt = 0);
	~Buffer();

	Buffer(const Buffer& other) = delete;
	Buffer& operator = (const Buffer& other) = delete;

	void bind() const;
	void unbind() const;
	void release();

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

	inline GLenum getTarget() const {
		return m_target;
	}

	inline GLenum getUsage() const {
		return m_usage;
	}

protected:
	GLuint m_handler;
	size_t m_size; //buffer size in bytes
	size_t m_elementCount; //element count in buffer
	GLenum m_target;
	GLenum m_usage;

};