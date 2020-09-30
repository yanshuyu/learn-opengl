#include"ShaderProgram.h"
#include"Shader.h"
#include<fstream>
#include<sstream>
#include<algorithm>
#include<iterator>
#include<memory>


ShaderProgram::ShaderProgram(const std::string& name, const std::string& file):m_name(name)
, m_file(file)
, m_handler(0)
, m_linked(false) {

}

ShaderProgram::~ShaderProgram() {
	release();
}

bool ShaderProgram::compileAndLink() {
	if (m_linked)
		return true;

	if (m_file.empty())
		return false;

	std::string vsrc;
	std::string fsrc;
	if (!parseShaderSource(vsrc, fsrc))
		return false;
	
#if _DEBUG
	std::stringstream msg;
	msg << "\"" << m_name << "\"" << " parsed vs: \n" << vsrc << std::endl;
	msg << "\"" << m_name << "\"" << " parsed fs: \n" << fsrc << std::endl;
	CONSOLELOG(msg.str());
#endif

	std::unique_ptr<Shader> vs = nullptr;
	std::unique_ptr<Shader> fs = nullptr;

	if (!vsrc.empty()) {
		vs = std::make_unique<Shader>(vsrc, Shader::Type::VertexShader);
		if (!vs->compile()) {
			std::stringstream msg;
			msg << "[Shader Load error] VS Compile error: " << vs->getInfoLog() << std::endl;
			CONSOLELOG(msg.str());
			return false;
		}
	}

	if (!fsrc.empty()) {
		fs = std::make_unique<Shader>(fsrc, Shader::Type::FragmentShader);
		if (!fs->compile()) {
			std::stringstream msg;
			msg << "[Shader Load error] FS Compile error: " << fs->getInfoLog() << std::endl;
			CONSOLELOG(msg.str());
			return false;
		}
	}

	GLCALL(m_handler = glCreateProgram());
	
	if (m_handler == 0)
		return false;

	if (vs) {
		GLCALL(glAttachShader(m_handler, vs->getHandler()));
	}
	if (fs) {
		GLCALL(glAttachShader(m_handler, fs->getHandler()));
	}
	
	GLCALL(glLinkProgram(m_handler));

	GLint linked = GL_FALSE;
	GLCALL(glGetProgramiv(m_handler, GL_LINK_STATUS, &linked));

	if (!linked) {
		std::stringstream msg;
		msg << "[Shader Link error] info log: " << getInfoLog() << std::endl;
		CONSOLELOG(msg.str());

#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		
		release();
		return false;
	}

	m_linked = true;
	
	queryProgramInfo();

#if _DEBUG 
	dumpProgramInfo();
#endif

	return true;
}


bool ShaderProgram::isLinked() const {
	return m_linked;
}


void ShaderProgram::release() {
	if (m_handler) {
		GLCALL(glDeleteProgram(m_handler));
		m_handler = 0;
		m_linked = false;
		m_attributes.clear();
		m_uniforms.clear();
		m_uniformBlocks.clear();
	}
}


GLuint ShaderProgram::getHandler() const {
	return m_handler;
}


std::string ShaderProgram::getInfoLog() const {
	if (m_handler == 0)
		return "";
	
	GLint len = 0;
	GLCALL(glGetProgramiv(m_handler, GL_INFO_LOG_LENGTH, &len));

	if (len <= 0)
		return "";

	GLchar* logBuffer = new GLchar[len];
	std::string log;
	GLCALL(glGetProgramInfoLog(m_handler, len, &len, logBuffer));
	log.assign(logBuffer);
	delete[] logBuffer;

	return log;
}


std::vector<ShaderProgram::Attribute> ShaderProgram::getAttributes() const {
	return m_attributes;
}


std::vector<ShaderProgram::Uniform> ShaderProgram::getUniforms() const {
	return m_uniforms;
}


std::vector<ShaderProgram::UniformBlock> ShaderProgram::getUniformBlocks() const {
	return m_uniformBlocks;
}


bool ShaderProgram::hasAttribute(const std::string& name) const {
	auto pos = std::find_if(m_attributes.begin(), m_attributes.end(), [&](const ShaderProgram::Attribute& attr) {
		return attr.name == name;
	});
	return pos != m_attributes.end();
}

bool ShaderProgram::hasUniform(const std::string& name) const {
	auto pos = std::find_if(m_uniforms.begin(), m_uniforms.end(), [&](const ShaderProgram::Uniform& u) {
		return u.name == name;
	});
	return pos != m_uniforms.end();
}

bool ShaderProgram::hasUniformBlock(const std::string& name) const {
	return getUniformBlockIndex(name) != -1;
}

void ShaderProgram::bind() const {
	GLCALL(glUseProgram(m_handler));
}

void ShaderProgram::unbind() const {
	GLCALL(glUseProgram(0));
}

bool ShaderProgram::bindUniformBlock(const std::string& name, UniformBlockBindingPoint bp) {
	int blockIdx = getUniformBlockIndex(name);
	if (blockIdx == -1)
		return false;

	GLCALL(glUniformBlockBinding(m_handler, blockIdx, int(bp)));
	return true;
}

void ShaderProgram::unbindUniformBlock(const std::string& name) {
	int blockIdx = getUniformBlockIndex(name);
	if (blockIdx != -1)
		GLCALL(glUniformBlockBinding(m_handler, blockIdx, 0));
}

bool ShaderProgram::parseShaderSource(std::string& vs, std::string& fs) {
	std::ifstream ifs(m_file);
	if (!ifs.is_open())
		return false;
	
	std::string line;
	std::stringstream srcs[2];
	int idx = -1;
	while (std::getline(ifs, line)) {
		if (line.find("#shader") != std::string::npos || line.find("#Shader") != std::string::npos) {
			if (line.find("vertex") != std::string::npos
				|| line.find("Vertex") != std::string::npos
				|| line.find("VERTEX") != std::string::npos
				|| line.find("vs") != std::string::npos) { // vertex shader session
				idx = 0;
				continue;
			} else if (line.find("fragment") != std::string::npos
				|| line.find("Fragment") != std::string::npos
				|| line.find("FRAGMENT") != std::string::npos
				|| line.find("fs") != std::string::npos) {
				idx = 1;
				continue;
			}
		}

		if (idx != -1)
			srcs[idx] << line << "\n";
	}

	ifs.close();
	vs.assign(srcs[0].str());
	fs.assign(srcs[1].str());

	return !vs.empty() || !fs.empty();
}


void ShaderProgram::queryProgramInfo() {
	// vertex attributes
	GLint attriCount = 0;
	GLint attriMaxLen = 0;
	GLCALL(glGetProgramiv(m_handler, GL_ACTIVE_ATTRIBUTES, &attriCount));
	GLCALL(glGetProgramiv(m_handler, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attriMaxLen));

	if (attriCount > 0) {
		GLchar* nameBuffer = new GLchar[attriMaxLen];
		GLint outNameLen = 0;
		GLenum attriType = GL_FLOAT;
		GLint attriSz = 0;
		
		m_attributes.reserve(attriCount);
		for (size_t i = 0; i < attriCount; ++i) {
			Attribute attr;
			outNameLen = 0;
			GLCALL(glGetActiveAttrib(m_handler,
								i, 
								attriMaxLen,
								&outNameLen,
								&attr.elementSize,
								&attr.elemenTtype,
								nameBuffer));
			attr.name.assign(nameBuffer, outNameLen);
			GLCALL(attr.location = glGetAttribLocation(m_handler, attr.name.c_str()));
			m_attributes.push_back(attr);
		}
		delete[] nameBuffer;
	}

	
	// uniforms
	GLint uniformCount = 0;
	GLint uniformMaxLen = 0;
	GLCALL(glGetProgramiv(m_handler, GL_ACTIVE_UNIFORMS, &uniformCount));
	GLCALL(glGetProgramiv(m_handler, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformMaxLen));

	if (uniformCount > 0) {
		GLchar* nameBuffer = new GLchar[uniformMaxLen];
		GLint outNameLen = 0;
		m_uniforms.reserve(uniformCount);

		for (size_t i = 0; i < uniformCount; ++i) {
			Uniform uniform;
			outNameLen = 0;

			GLCALL(glGetActiveUniform(m_handler,
									i,
									uniformMaxLen,
									&outNameLen,
									&uniform.elementSize,
									&uniform.elemenTtype,
									nameBuffer));
			uniform.name.assign(nameBuffer, outNameLen);
			GLCALL(uniform.location = glGetUniformLocation(m_handler, uniform.name.c_str()));
			m_uniforms.push_back(uniform);
		}
		delete[] nameBuffer;
	}


	// uniform block
	GLint ubCount = 0;
	GLint ubMaxLen = 0;
	GLCALL(glGetProgramiv(m_handler, GL_ACTIVE_UNIFORM_BLOCKS, &ubCount));
	GLCALL(glGetProgramiv(m_handler, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &ubMaxLen));

	if (ubCount > 0) {
		GLchar* nameBuffer = new GLchar[ubMaxLen];
		GLint outNameLen = 0;
		m_uniformBlocks.reserve(ubCount);

		for (size_t i = 0; i < ubCount; ++i) {
			UniformBlock ub;
			outNameLen = 0;

			GLCALL(glGetActiveUniformBlockName(m_handler,
											i,
											ubMaxLen,
											&outNameLen,
											nameBuffer));
			ub.name.assign(nameBuffer, outNameLen);
			GLCALL(ub.index = glGetUniformBlockIndex(m_handler, ub.name.c_str()));
			GLCALL(glGetActiveUniformBlockiv(m_handler, i, GL_UNIFORM_BLOCK_DATA_SIZE, &ub.dataSize));

			GLint compUniformCount = 0;
			GLCALL(glGetActiveUniformBlockiv(m_handler, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &compUniformCount));
			ub.uniformIndices.resize(compUniformCount);
			GLCALL(glGetActiveUniformBlockiv(m_handler, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, &ub.uniformIndices[0]));

			m_uniformBlocks.push_back(ub);
		}
		delete[] nameBuffer;
	}
	
}


int ShaderProgram::getUniformLocation(const std::string& name) const {
	auto pos = std::find_if(m_uniforms.begin(), m_uniforms.end(), [&](const Uniform& u) {
			return u.name == name;
		});

	if (pos == m_uniforms.end())
		return -1;

	return pos->location;
}


int ShaderProgram::getUniformBlockIndex(const std::string& name) const {
	auto pos = std::find_if(m_uniformBlocks.begin(), m_uniformBlocks.end(), [&](const ShaderProgram::UniformBlock& ub) {
		return ub.name == name;
	});

	if (pos == m_uniformBlocks.end())
		return -1;
	
	return pos->index;
}

void ShaderProgram::dumpProgramInfo() const {
	std::cout << "Shader program \"" << m_name <<"(" << m_handler << ")" << "\" info:\n" << "\tattributes: [ ";
	std::ostream_iterator<Attribute> aitr(std::cout, "\n");
	std::copy(m_attributes.begin(), m_attributes.end(), aitr);
	
	std::cout << " ]\n" << "\tuniforms: [ ";
	std::ostream_iterator<Uniform> uitr(std::cout, "\n");
	std::copy(m_uniforms.begin(), m_uniforms.end(), uitr);

	std::cout << " ]\n" << "\tuniformBlocks: [";
	std::ostream_iterator<UniformBlock> ubitr(std::cout, "\n");
	std::copy(m_uniformBlocks.begin(), m_uniformBlocks.end(), ubitr);
	std::cout << " ]\n" << std::endl;
}


template<>
bool ShaderProgram::setUniform1<float>(const std::string& name, float f1) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform1f(location, f1));
	return true;
}

template<>
bool ShaderProgram::setUniform1<double>(const std::string& name, double d1) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform1d(location, d1));
	return true;
}

template<>
bool ShaderProgram::setUniform1<int>(const std::string& name, int i1) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform1i(location, i1));
	return true;
}




template<>
bool ShaderProgram::setUniform2<float>(const std::string& name, float f1, float f2) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform2f(location, f1, f2));
	return true;
}

template<>
bool ShaderProgram::setUniform2<double>(const std::string& name, double d1, double d2) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform2d(location, d1, d2));
	return true;
}


template<>
bool ShaderProgram::setUniform2<int>(const std::string& name, int i1, int i2) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform2i(location, i1, i2));
	return true;
}




template<>
bool ShaderProgram::setUniform3<float>(const std::string& name, float f1, float f2, float f3) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform3f(location, f1, f2, f3));
	return true;
}

template<>
bool ShaderProgram::setUniform3<double>(const std::string& name, double d1, double d2, double d3) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform3d(location, d1, d2, d3));
	return true;
}

template<>
bool ShaderProgram::setUniform3<int>(const std::string& name, int i1, int i2, int i3) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform3i(location, i1, i2, i3));
	return true;
}


template<>
bool ShaderProgram::setUniform4<float>(const std::string& name, float f1, float f2, float f3, float f4) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform4f(location, f1, f2, f3, f4));
	return true;
}

template<>
bool ShaderProgram::setUniform4<double>(const std::string& name, double d1, double d2, double d3, double d4) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform4d(location, d1, d2, d3, d4));
	return true;
}

template<>
bool ShaderProgram::setUniform4<int>(const std::string& name, int i1, int i2, int i3, int i4) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform4i(location, i1, i2, i3, i4));
	return true;
}



template<>
bool ShaderProgram::setUniform1v<float>(const std::string& name, float* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform1fv(location, count, data));
	return true;
};


template<>
bool ShaderProgram::setUniform1v<double>(const std::string& name, double* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform1dv(location, count, data));
	return true;
};


template<>
bool ShaderProgram::setUniform1v<int>(const std::string& name, int* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform1iv(location, count, data));
	return true;
};


template<>
bool ShaderProgram::setUniform2v<float>(const std::string& name, float* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform2fv(location, count, data));
	return true;
};

template<>
bool ShaderProgram::setUniform2v<double>(const std::string& name, double* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform2dv(location, count, data));
	return true;
};

template<>
bool ShaderProgram::setUniform2v<int>(const std::string& name, int* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform2iv(location, count, data));
	return true;
};

template<>
bool ShaderProgram::setUniform3v<float>(const std::string& name, float* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform3fv(location, count, data));
	return true;
};

template<>
bool ShaderProgram::setUniform3v<double>(const std::string& name, double* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform3dv(location, count, data));
	return true;
};

template<>
bool ShaderProgram::setUniform3v<int>(const std::string& name, int* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform3iv(location, count, data));
	return true;
};


template<>
bool ShaderProgram::setUniform4v<float>(const std::string& name, float* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform4fv(location, count, data));
	return true;
};


template<>
bool ShaderProgram::setUniform4v<double>(const std::string& name, double* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform4dv(location, count, data));
	return true;
};


template<>
bool ShaderProgram::setUniform4v<int>(const std::string& name, int* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform4iv(location, count, data));
	return true;
};


template<>
bool ShaderProgram::setUniformMat4v<float>(const std::string& name, float* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;
	GLCALL(glUniformMatrix4fv(location, count, false, data));
}

template<>
bool ShaderProgram::setUniformMat4v<double>(const std::string& name, double* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;
	GLCALL(glUniformMatrix4dv(location, count, false, data));
}


std::ostream& operator << (std::ostream& o, const ShaderProgram::Attribute& a) {
	o << "{ name: " << a.name << ", location: " << a.location << ", type: " << a.elemenTtype << ", size: " << a.elementSize << " }";
	return o;
}


std::ostream& operator << (std::ostream& o, const ShaderProgram::UniformBlock& ub) {
	o << "{ name: " << ub.name << ", index: " << ub.index << ", dataSize: " << ub.dataSize << ", uniformIndices: [ ";
	std::ostream_iterator<int> oitr(o, ", ");
	std::copy(ub.uniformIndices.begin(), ub.uniformIndices.end(), oitr);
	o << " ] }";
	return o;
}


