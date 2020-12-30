#include"ShaderProgram.h"
#include"Util.h"
#include"FrameAllocator.h"
#include"Containers.h"
#include<filesystem>
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

	auto shaderSrcs = parseShaderSource(m_file);
	if (shaderSrcs.empty())
		return false;

	GLCALL(m_handler = glCreateProgram());
	if (m_handler == 0)
		return false;

	if (shaderSrcs.find(Shader::Type::ComputeShader) != shaderSrcs.end()) { // compute shader can't link with other shader type
		for (auto pos = shaderSrcs.begin(); pos != shaderSrcs.end(); ) {
			if (pos->first != Shader::Type::ComputeShader)
				pos = shaderSrcs.erase(pos);
			else
				pos++;
		}
	}

	for (auto& src : shaderSrcs) {
		Shader shader(src.second, src.first);
		if (!shader.compile()) {
			std::stringstream msg;
			msg << "Program \"" << m_file << "\" " << ShaderType2Str(src.first) << " Compile error, log: " << shader.getInfoLog() << std::endl;
			CONSOLELOG(msg.str());
			return false;
		}

		GLCALL(glAttachShader(m_handler, shader.getHandler()));
	}

	GLCALL(glLinkProgram(m_handler));

	GLint linked = GL_FALSE;
	GLCALL(glGetProgramiv(m_handler, GL_LINK_STATUS, &linked));

	if (!linked) {
		std::stringstream msg;
		msg << "Program \"" << m_file << "\"" << "Link error, log: " << getInfoLog() << std::endl;
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
	//dumpProgramInfo();
	std::cout << *this << std::endl;
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
		m_shaderStorageBlocks.clear();
		m_stageSubroutinesInfo.clear();
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

bool ShaderProgram::hasShaderStorageBlock(const std::string& name) const {
	return getShaderStorageBlockIndex(name) != -1;
}

bool ShaderProgram::hasSubroutineUniform(Shader::Type shaderStage, const std::string& name) const {
	return getSubroutineUniform(shaderStage, name) != nullptr;
}


void ShaderProgram::bind() const {
	GLCALL(glUseProgram(m_handler));
}

void ShaderProgram::unbind() const {
	GLCALL(glUseProgram(0));
}

bool ShaderProgram::isBinded() const {
	int currendId = 0;
	GLCALL(glGetIntegerv(GL_CURRENT_PROGRAM, &currendId));
	return currendId == m_handler;
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

bool ShaderProgram::bindShaderStorageBlock(const std::string& name, size_t idx) {
	int blockIdx = getShaderStorageBlockIndex(name);
	if (blockIdx == -1)
		return false;

	GLCALL(glShaderStorageBlockBinding(m_handler, blockIdx, idx));
	return true;
}

void ShaderProgram::unbindShaderStorageBlock(const std::string& name) {
	int blockIdx = getShaderStorageBlockIndex(name);
	if (blockIdx != -1)
		GLCALL(glShaderStorageBlockBinding(m_handler, blockIdx, 0));
}


bool ShaderProgram::setSubroutineUniform(Shader::Type stage, const std::string& uniformName, const std::string& subrotineName) {
	auto uniform = getSubroutineUniform(stage, uniformName);
	if (!uniform)
		return false;
	
	auto subrotine = getSubroutine(stage, subrotineName);
	if (!subrotine)
		return false;

	if (!checkSubroutineCompatible(uniform, subrotine))
		return false;

	if (m_subroutineMapping.find(stage) == m_subroutineMapping.end())
		m_subroutineMapping[stage] = {};
	
	m_subroutineMapping.at(stage)[uniformName] = subrotineName;

	return true;
}


void ShaderProgram::bindSubroutineUniforms() {
	if (!isBinded())
		return;

	frame_mark();

	for (auto& suMapping : m_subroutineMapping) {
		if (suMapping.second.empty())
			continue;

		FrameVector<std::pair<int, int>> locationIndexMapping;
		locationIndexMapping.reserve(suMapping.second.size());
		for (auto& nameMapping : suMapping.second) {
			SubroutineUniform* su = getSubroutineUniform(suMapping.first, nameMapping.first);
			Subroutine* st = getSubroutine(suMapping.first, nameMapping.second);
			locationIndexMapping.push_back({ su->location, st->index });
		}

		std::sort(locationIndexMapping.begin(),
			locationIndexMapping.end(),
			[](decltype(locationIndexMapping)::const_reference l, decltype(locationIndexMapping)::const_reference r) {
				return l.first < r.first;
			});

		FrameVector<int> indices;
		indices.reserve(locationIndexMapping.size());
		std::transform(locationIndexMapping.begin(),
			locationIndexMapping.end(),
			std::back_inserter<decltype(indices)>(indices), [](decltype(locationIndexMapping)::const_reference locIdx) {
				return locIdx.second;
			});

		GLCALL(glUniformSubroutinesuiv(int(suMapping.first), indices.size(), reinterpret_cast<unsigned*>(indices.data())));
	}

	frame_clear();
}


void ShaderProgram::unbindSubroutineUniforms() {
	for (auto& suMapping : m_subroutineMapping) {
		suMapping.second.clear();
	}
}

const std::vector<ShaderProgram::Subroutine>& ShaderProgram::getSubroutines(Shader::Type shaderStage) const {
	auto info = m_stageSubroutinesInfo.find(shaderStage);
	if (info == m_stageSubroutinesInfo.end()) {
		m_stageSubroutinesInfo[shaderStage] = StageSubroutineInfo();
		return m_stageSubroutinesInfo[shaderStage].subroutines;
	}

	return info->second.subroutines;
}

const std::vector<ShaderProgram::SubroutineUniform>& ShaderProgram::getSubroutineUniforms(Shader::Type shaderStage) const {
	auto info = m_stageSubroutinesInfo.find(shaderStage);
	if (info == m_stageSubroutinesInfo.end()) {
		m_stageSubroutinesInfo[shaderStage] = StageSubroutineInfo();
		return m_stageSubroutinesInfo[shaderStage].subroutineUniforms;
	}

	return info->second.subroutineUniforms;
}


std::unordered_map<Shader::Type, std::string> ShaderProgram::parseShaderSource(const std::string& file) {
	std::ifstream ifs(file);
	if (!ifs.is_open())
		return {};

	std::string line;
	std::stringstream srcSessions[4];
	int idx = -1;
	while (std::getline(ifs, line)) {
		if (line.find("#shader") != std::string::npos || line.find("#Shader") != std::string::npos) {
			if (line.find("vertex") != std::string::npos
				|| line.find("Vertex") != std::string::npos
				|| line.find("VERTEX") != std::string::npos
				|| line.find("vs") != std::string::npos
				|| line.find("VS") != std::string::npos) { // vertex shader session
				idx = 0;
				continue;
			}
			else if (line.find("geometry") != std::string::npos // geometry shader session
				|| line.find("Geometry") != std::string::npos
				|| line.find("GEOMETRY") != std::string::npos
				|| line.find("gs") != std::string::npos
				|| line.find("GS") != std::string::npos) {
				idx = 1;
				continue;
			}
			else if (line.find("fragment") != std::string::npos // fragment shader session
				|| line.find("Fragment") != std::string::npos
				|| line.find("FRAGMENT") != std::string::npos
				|| line.find("fs") != std::string::npos
				|| line.find("FS") != std::string::npos) {
				idx = 2;
				continue;
			}
			else if (line.find("compute") != std::string::npos // compute shader session
				|| line.find("Compute") != std::string::npos
				|| line.find("COMPUTE") != std::string::npos
				|| line.find("cs") != std::string::npos
				|| line.find("CS") != std::string::npos) {
				idx = 3;
				continue;
			}
		}

		if (line.find("#include") != std::string::npos) { // include another glsl source file session
#ifdef _DEBUG
			ASSERT(idx != -1);
#endif // _DEBUG

			line = parseIncludedSource(line);
		}

		if (idx != -1)
			srcSessions[idx] << line << "\n";
	}

	ifs.close();
	
	std::unordered_map<Shader::Type, std::string> shaderSources;
	std::string vs(srcSessions[0].str());
	std::string gs(srcSessions[1].str());
	std::string fs(srcSessions[2].str());
	std::string cs(srcSessions[3].str());
	
	if (!vs.empty())
		shaderSources[Shader::Type::VertexShader] = std::move(vs);

	if (!gs.empty())
		shaderSources[Shader::Type::GeometryShader] = std::move(gs);

	if (!fs.empty())
		shaderSources[Shader::Type::FragmentShader] = std::move(fs);

	if (!cs.empty())
		shaderSources[Shader::Type::ComputeShader] = std::move(cs);

	return shaderSources;
}


std::string ShaderProgram::parseIncludedSource(const std::string& includeExp) {
	auto beg = includeExp.find_first_of('"');
	auto end = includeExp.find_last_of('"');
	if (beg == std::string::npos || end == std::string::npos)
		return "";
		
	std::string relPath = includeExp.substr(beg + 1, end - beg - 1);
	std::filesystem::path curPath(m_file);
	curPath = curPath.parent_path();
	curPath /= relPath;

	if (!std::filesystem::exists(curPath)) {
		std::stringstream err;
		err << "Included file \"" << curPath.string() << "\" not exit!" << "\n";
		CONSOLELOG(err.str());
		return "";
	}

	std::ifstream fs(curPath.string());
	if (!fs.is_open()) {
		std::stringstream err;
		err << "Failed to open Included file \"" << curPath.string() << "\"" << "\n" ;
		CONSOLELOG(err.str());
		return "";
	}

	std::stringstream srcBuffer;
	std::string line;
	while (std::getline(fs, line)) {
		if (line.find("#include") != std::string::npos) {
			line = parseIncludedSource(line);
		}

		srcBuffer << line << "\n";
	}

	fs.close();
	
	return srcBuffer.str();
}


void ShaderProgram::queryProgramInfo() {
	// vertex attributes
	GLint attriCount = 0;
	GLint attriMaxLen = 0;
	GLCALL(glGetProgramiv(m_handler, GL_ACTIVE_ATTRIBUTES, &attriCount));
	GLCALL(glGetProgramiv(m_handler, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attriMaxLen));
	if (attriCount > 0) {
		std::vector<char> nameBuffer(attriMaxLen);
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
								&attr.elementType,
								&nameBuffer[0]));
			attr.name.assign(nameBuffer.data(), outNameLen);
			GLCALL(attr.location = glGetAttribLocation(m_handler, attr.name.c_str()));
			m_attributes.push_back(attr);
		}
	}

	
	// uniforms
	GLint uniformCount = 0;
	GLint uniformMaxLen = 0;
	GLCALL(glGetProgramiv(m_handler, GL_ACTIVE_UNIFORMS, &uniformCount));
	GLCALL(glGetProgramiv(m_handler, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformMaxLen));
	if (uniformCount > 0) {
		std::vector<char> nameBuffer(uniformMaxLen);
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
									&uniform.elementType,
									&nameBuffer[0]));
			uniform.name.assign(nameBuffer.data(), outNameLen);
			GLCALL(uniform.location = glGetUniformLocation(m_handler, uniform.name.c_str()));
			
			if (uniform.location < 0)
				continue;

			m_uniforms.push_back(uniform);
		}
	}


	// uniform block
	GLint ubCount = 0;
	GLint ubMaxLen = 0;
	GLCALL(glGetProgramiv(m_handler, GL_ACTIVE_UNIFORM_BLOCKS, &ubCount));
	GLCALL(glGetProgramiv(m_handler, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &ubMaxLen));
	if (ubCount > 0) {
		std::vector<char> nameBuffer(ubMaxLen);
		GLint outNameLen = 0;
		m_uniformBlocks.reserve(ubCount);

		for (size_t i = 0; i < ubCount; ++i) {
			UniformBlock ub;
			outNameLen = 0;
			ub.index = i;

			GLCALL(glGetActiveUniformBlockName(m_handler,
											i,
											ubMaxLen,
											&outNameLen,
											&nameBuffer[0]));
			ub.name.assign(nameBuffer.data(), outNameLen);

			GLCALL(glGetActiveUniformBlockiv(m_handler, i, GL_UNIFORM_BLOCK_DATA_SIZE, &ub.dataSize));

			GLint uniformCount = 0;
			GLCALL(glGetActiveUniformBlockiv(m_handler, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &uniformCount));
			if (uniformCount <= 0)
				continue;

			ub.uniforms.reserve(uniformCount);
			std::vector<int> uniformIndices(uniformCount);
			GLCALL(glGetActiveUniformBlockiv(m_handler, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, &uniformIndices[0]));
			
			std::vector<char> nameBuffer(uniformMaxLen);
			GLint outNameLen = 0;
			for (auto idx : uniformIndices) {
				Uniform uniform;
				outNameLen = 0;
				GLCALL(glGetActiveUniform(m_handler, idx, uniformMaxLen, &outNameLen, &uniform.elementSize, &uniform.elementType, &nameBuffer[0]));
				uniform.name.assign(nameBuffer.data(), outNameLen);
				ub.uniforms.push_back(uniform);
			}

			m_uniformBlocks.push_back(ub);
		}
	}
	

	// subroutines and subroutineuniforms
	Shader::Type shaderStages[] = { Shader::Type::VertexShader, Shader::Type::GeometryShader, Shader::Type::FragmentShader, Shader::Type::ComputeShader };
	for (auto stage : shaderStages) {
		StageSubroutineInfo stInfo;
		stInfo.stage = stage;

		int activeStCount = 0;
		int maxStNameLen = 0;
		GLCALL(glGetProgramStageiv(m_handler, int(stage), GL_ACTIVE_SUBROUTINES, &activeStCount));
		GLCALL(glGetProgramStageiv(m_handler, int(stage), GL_ACTIVE_SUBROUTINE_MAX_LENGTH, &maxStNameLen));

		if (activeStCount > 0) {
			std::string name;
			int len = 0;
			
			for (size_t i = 0; i < activeStCount; i++) {
				name.clear();
				name.resize(maxStNameLen);
				len = 0;
				Subroutine st;

				st.shaderStage = stage;
				GLCALL(glGetActiveSubroutineName(m_handler, int(stage), i, maxStNameLen, &len, const_cast<char*>(name.data())));
				st.name = name.substr(0, len);
				GLCALL(st.index = glGetSubroutineIndex(m_handler, int(stage), st.name.data()));
				
				stInfo.subroutines.push_back(st);
			}
		}

		int activeSuCount = 0;
		int maxSuNameLen = 0;
		GLCALL(glGetProgramStageiv(m_handler, int(stage), GL_ACTIVE_SUBROUTINE_UNIFORMS, &activeSuCount));
		GLCALL(glGetProgramStageiv(m_handler, int(stage), GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH, &maxSuNameLen));

		if (activeSuCount > 0) {
			std::string name;
			int len = 0;
			
			for (size_t j = 0; j < activeSuCount; j++) {
				name.clear();
				name.resize(maxSuNameLen);
				len = 0;
				SubroutineUniform su;

				su.shaderStage = stage;
				GLCALL(glGetActiveSubroutineUniformName(m_handler, int(stage), j, maxSuNameLen, &len, const_cast<char*>(name.data())));
				su.name = name.substr(0, len);
				GLCALL(su.location = glGetSubroutineUniformLocation(m_handler, int(stage), su.name.data()));
				int val = 0;
				GLCALL(glGetActiveSubroutineUniformiv(m_handler, int(stage), j, GL_UNIFORM_SIZE, &val));
				su.size = val;
				GLCALL(glGetActiveSubroutineUniformiv(m_handler, int(stage), j, GL_NUM_COMPATIBLE_SUBROUTINES, &val));
				su.compatibleSubroutineIndices.resize(val, -1);
				GLCALL(glGetActiveSubroutineUniformiv(m_handler, int(stage), j, GL_COMPATIBLE_SUBROUTINES, su.compatibleSubroutineIndices.data()));

				stInfo.subroutineUniforms.push_back(su);
			}
		}

		if (activeStCount > 0 || activeSuCount > 0)
			m_stageSubroutinesInfo[Shader::Type(stage)] = stInfo;
	}

	
	// shader storage buffer (new shader interface query api)
	int numSSBO = 0;
	int ssboMaxNameLen = 0;
	GLCALL(glGetProgramInterfaceiv(m_handler, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &numSSBO));
	GLCALL(glGetProgramInterfaceiv(m_handler, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &ssboMaxNameLen));
	if (numSSBO > 0) {
		std::vector<char> nameBuffer(ssboMaxNameLen);
		for (size_t i = 0; i < numSSBO; i++) {
			ShaderStorageBlock ssbo;
			ssbo.index = i;

			GLsizei nameLen = 0;
			GLCALL(glGetProgramResourceName(m_handler, GL_SHADER_STORAGE_BLOCK, i, ssboMaxNameLen, &nameLen, &nameBuffer[0]));
			ssbo.name.assign(nameBuffer.data(), nameLen);

			GLenum dataSizeProp = GL_BUFFER_DATA_SIZE;
			GLCALL(glGetProgramResourceiv(m_handler, GL_SHADER_STORAGE_BLOCK, i, 1, &dataSizeProp, 1, nullptr, &ssbo.dataSize));

			m_shaderStorageBlocks.push_back(ssbo);
		}
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


int ShaderProgram::getShaderStorageBlockIndex(const std::string& name) const {
	auto pos = std::find_if(m_shaderStorageBlocks.begin(), m_shaderStorageBlocks.end(), [&](const ShaderStorageBlock& ssbo) {
		return ssbo.name == name;
		});

	if (pos == m_shaderStorageBlocks.end())
		return -1;

	return pos->index;
}


ShaderProgram::SubroutineUniform* ShaderProgram::getSubroutineUniform(Shader::Type shaderStage, const std::string& name) const {
	auto info = m_stageSubroutinesInfo.find(shaderStage);
	if (info == m_stageSubroutinesInfo.end())
		return nullptr;
	
	auto& suVec = info->second.subroutineUniforms;
	auto su = std::find_if(suVec.begin(), suVec.end(), [&](const SubroutineUniform& _su) {
		return _su.name == name;
	});

	if (su == suVec.end())
		return nullptr;

	return &(*su);
}

ShaderProgram::Subroutine* ShaderProgram::getSubroutine(Shader::Type shaderStage, const std::string& name) const {
	auto info = m_stageSubroutinesInfo.find(shaderStage);
	if (info == m_stageSubroutinesInfo.end())
		return nullptr;

	auto& stVec = info->second.subroutines;
	auto st = std::find_if(stVec.begin(), stVec.end(), [&](const Subroutine& _st) {
		return _st.name == name;
	});

	if (st == stVec.end())
		return nullptr;

	return &(*st);
}

bool ShaderProgram::checkSubroutineCompatible(SubroutineUniform* su, Subroutine* st) const {
	auto founded = std::find(su->compatibleSubroutineIndices.begin(), su->compatibleSubroutineIndices.end(), st->index);
	return founded != su->compatibleSubroutineIndices.end();
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
bool ShaderProgram::setUniform1<unsigned>(const std::string& name, unsigned ui1) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform1ui(location, ui1));
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
bool ShaderProgram::setUniform1v<float>(const std::string& name, const float* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform1fv(location, count, data));
	return true;
};


template<>
bool ShaderProgram::setUniform1v<double>(const std::string& name, const double* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform1dv(location, count, data));
	return true;
};


template<>
bool ShaderProgram::setUniform1v<int>(const std::string& name, const int* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform1iv(location, count, data));
	return true;
};


template<>
bool ShaderProgram::setUniform2v<float>(const std::string& name, const float* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform2fv(location, count, data));
	return true;
};

template<>
bool ShaderProgram::setUniform2v<double>(const std::string& name, const double* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform2dv(location, count, data));
	return true;
};

template<>
bool ShaderProgram::setUniform2v<int>(const std::string& name, const int* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform2iv(location, count, data));
	return true;
};

template<>
bool ShaderProgram::setUniform3v<float>(const std::string& name, const float* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform3fv(location, count, data));
	return true;
};

template<>
bool ShaderProgram::setUniform3v<double>(const std::string& name, const double* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform3dv(location, count, data));
	return true;
};

template<>
bool ShaderProgram::setUniform3v<int>(const std::string& name, const int* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform3iv(location, count, data));
	return true;
};


template<>
bool ShaderProgram::setUniform4v<float>(const std::string& name, const float* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform4fv(location, count, data));
	return true;
};


template<>
bool ShaderProgram::setUniform4v<double>(const std::string& name, const double* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform4dv(location, count, data));
	return true;
};


template<>
bool ShaderProgram::setUniform4v<int>(const std::string& name, const int* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;

	GLCALL(glUniform4iv(location, count, data));
	return true;
};


template<>
bool ShaderProgram::setUniformMat4v<float>(const std::string& name, const float* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;
	GLCALL(glUniformMatrix4fv(location, count, false, data));
}

template<>
bool ShaderProgram::setUniformMat4v<double>(const std::string& name, const double* data, size_t count) const {
	int location = getUniformLocation(name);
	if (location == -1)
		return false;
	GLCALL(glUniformMatrix4dv(location, count, false, data));
}


std::ostream& operator << (std::ostream& o, const ShaderProgram::Attribute& a) {
	o << "{ name: " << a.name << ", location: " << a.location << ", type: " << a.elementType << ", size: " << a.elementSize << " }";
	return o;
}


std::ostream& operator << (std::ostream& o, const ShaderProgram::UniformBlock& ub) {
	o << "{ name: " << ub.name << ", index: " << ub.index << ", dataSize: " << ub.dataSize << ", uniforms: [ ";
	std::ostream_iterator<ShaderProgram::Uniform> oitr(o, ", ");
	std::copy(ub.uniforms.begin(), ub.uniforms.end(), oitr);
	o << " ] }";
	return o;
}


std::ostream& operator << (std::ostream& o, const ShaderProgram::Subroutine& st) {
	o << "{ name: " << st.name << ", index: " << st.index << ", stage: " << ShaderType2Str(st.shaderStage) << " }";
	return o;
}

std::ostream& operator << (std::ostream& o, const ShaderProgram::SubroutineUniform& su) {
	o << "{ name: " << su.name << ", location: " << su.location << ", stage: " << ShaderType2Str(su.shaderStage) << ", compatible suroutines: [";
	std::ostream_iterator<int> ositr(o, ", ");
	std::copy(su.compatibleSubroutineIndices.begin(), su.compatibleSubroutineIndices.end(), ositr);
	o << " ]";
	return o;
}

std::ostream& operator << (std::ostream& o, const ShaderProgram& program) {
	o << "Shader program \"" << program.m_name << "(" << program.m_handler << ")" << "\" Interface info:{\n" << "\tattributes: [ ";
	std::ostream_iterator<ShaderProgram::Attribute> aitr(o, "\n");
	std::copy(program.m_attributes.begin(), program.m_attributes.end(), aitr);

	o << " ]\n" << "\tuniforms: [ ";
	std::ostream_iterator<ShaderProgram::Uniform> uitr(o, "\n");
	std::copy(program.m_uniforms.begin(), program.m_uniforms.end(), uitr);

	std::cout << " ]\n" << "\tuniformBlocks: [";
	std::ostream_iterator<ShaderProgram::UniformBlock> ubitr(o, "\n");
	std::copy(program.m_uniformBlocks.begin(), program.m_uniformBlocks.end(), ubitr);
	
	o << " ]\n" << "\t stageInfo: [\n";
	for (auto& stInfo : program.m_stageSubroutinesInfo) {
		o << "subroutines: \n";
		std::ostream_iterator<ShaderProgram::Subroutine> stitr(o, "\n");
		std::copy(stInfo.second.subroutines.begin(), stInfo.second.subroutines.end(), stitr);

		o << "subroutine uniforms: \n";
		std::ostream_iterator<ShaderProgram::SubroutineUniform> suitr(o, "\n");
		std::copy(stInfo.second.subroutineUniforms.begin(), stInfo.second.subroutineUniforms.end(), suitr);
	}
	o << " ]\n";
	o << "}";

	return o;
}


