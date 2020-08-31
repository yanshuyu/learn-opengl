#pragma once
#include<glad/glad.h>
#include<string>


class Shader {
public:
	enum class Type {
		VertexShader,
		FragmentShader,
	};

public:
	Shader(const std::string& src, Type type = Type::VertexShader);
	~Shader();

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