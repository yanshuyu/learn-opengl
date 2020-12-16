#include"Shader.h"
#include"Util.h"

Shader::Shader(const std::string& src, Type type) :m_src(src)
, m_type(type)
, m_handler(0)
, m_compiled(false) {

}

Shader::~Shader() {
	release();
}

bool Shader::isCompiled() const {
	return m_compiled;
}

bool Shader::compile() {
	if (m_compiled)
		return true;

	if (m_type == Type::Unknown)
		return false;

	//GLenum shaderType = m_type == Type::VertexShader ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
	GLCALL(m_handler = glCreateShader(int(m_type)));

	if (!m_handler) 
		return false;
	
	const GLchar* srcBytes = m_src.c_str();
	GLint len = m_src.size();
	GLCALL(glShaderSource(m_handler, 1, &srcBytes, &len));
	GLCALL(glCompileShader(m_handler));


	GLint compiled = GL_FALSE;
	GLCALL(glGetShaderiv(m_handler, GL_COMPILE_STATUS, &compiled));
	if (!compiled) {
		return false;
	}

	m_compiled = true;
	return true;
}


std::string Shader::getSource() const {
	return m_src;
}


std::string Shader::getInfoLog() const {
	GLint len = 0;
	GLCALL(glGetShaderiv(m_handler, GL_INFO_LOG_LENGTH, &len));

	if (len <= 0)
		return "";

	GLchar* logBuffer = new GLchar[len];
	std::string log;
	GLCALL(glGetShaderInfoLog(m_handler, len, &len, logBuffer));
	log.assign(logBuffer);
	delete[] logBuffer;

	return log;
}

GLuint Shader::getHandler() const {
	return m_handler;
}


void Shader::release() {
	if (m_handler) {
		GLCALL(glDeleteShader(m_handler));
		m_handler = 0;
		m_compiled = false;
	}
}



std::string ShaderType2Str(Shader::Type shaderType) {
	switch (shaderType)
	{
	case Shader::Type::VertexShader:
		return "VertexShader";

	case Shader::Type::GeometryShader:
		return "GeometryShader";

	case Shader::Type::FragmentShader:
		return "FragmentShader";
	
	case Shader::Type::ComputeShader:
		return "ComputeShader";

	default:
		return "Unknown";
	}
}