#include"ShaderProgamMgr.h"
#include"Util.h"
#include<iostream>




std::shared_ptr<ShaderProgram> ShaderProgramManager::addProgram(const std::string& file, const std::string& name) {
	std::string shaderName(name);
	if (shaderName.empty())
		if (!ExtractFileNameFromPath(file, shaderName, false))
			shaderName = file;
	
	auto program = getProgram(shaderName);

	if (program != nullptr)
		return program;

	program = std::make_shared<ShaderProgram>(shaderName, file);
	if (program->compileAndLink()) {
		m_shaderPrograms.insert(std::make_pair(shaderName, program));
		return program;
	}

	return nullptr;
}


bool ShaderProgramManager::hasProgram(const std::string& name) const {
	return getProgram(name) != nullptr;
}

std::shared_ptr<ShaderProgram> ShaderProgramManager::getProgram(const std::string name) const {
	auto pos = m_shaderPrograms.find(name);
	if (pos == m_shaderPrograms.end())
		return nullptr;
	
	return pos->second;
}

std::shared_ptr<ShaderProgram> ShaderProgramManager::removeProgram(const std::string& name) {
	auto pos = m_shaderPrograms.find(name);
	if (pos != m_shaderPrograms.end()) {
		m_shaderPrograms.erase(pos);
		return pos->second;
	}
	return nullptr;
}