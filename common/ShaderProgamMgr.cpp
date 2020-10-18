#include"ShaderProgamMgr.h"
#include"Util.h"
#include<iostream>




std::weak_ptr<ShaderProgram> ShaderProgramManager::addProgram(const std::string& file, const std::string& name) {
	std::string shaderName(name);
	if (shaderName.empty())
		shaderName = ExtractFileNameFromPath(file, false);
	if (shaderName.empty())
		shaderName = file;
	
	auto found = getProgram(shaderName);
	if (!found.expired())
		return found;

	auto program = std::make_shared<ShaderProgram>(shaderName, file);
	if (program->compileAndLink()) {
		m_shaderPrograms.insert(std::make_pair(shaderName, program));
		return std::weak_ptr<ShaderProgram>(program);
	}

	return std::weak_ptr<ShaderProgram>();
}


bool ShaderProgramManager::hasProgram(const std::string& name) const {
	return !getProgram(name).expired();
}

std::weak_ptr<ShaderProgram> ShaderProgramManager::getProgram(const std::string name) const {
	auto pos = m_shaderPrograms.find(name);
	if (pos == m_shaderPrograms.end())
		return std::weak_ptr<ShaderProgram>();
	
	return std::weak_ptr<ShaderProgram>(pos->second);
}

bool ShaderProgramManager::removeProgram(const std::string& name) {
	auto pos = m_shaderPrograms.find(name);
	if (pos != m_shaderPrograms.end()) {
		m_shaderPrograms.erase(pos);
		return true;
	}
	return false;
}