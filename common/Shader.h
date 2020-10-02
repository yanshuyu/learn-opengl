#pragma once
#include<glad/glad.h>
#include<string>


class Shader {
public:
	enum class Type {
		Unknown,
		VertexShader = GL_VERTEX_SHADER,
		GeometryShader = GL_GEOMETRY_SHADER,
		FragmentShader = GL_FRAGMENT_SHADER,
	};

public:
	Shader(const std::string& src, Type type = Type::Unknown);
	~Shader();

	Shader(const Shader& other) = delete;
	Shader& operator = (const Shader& other) = delete;

	bool isCompiled() const;
	bool compile();
	void release();

	std::string getSource() const;
	std::string getInfoLog() const;
	GLuint getHandler() const;

protected:
	GLuint m_handler;
	Type m_type;
	bool m_compiled;
	std::string m_src;
};