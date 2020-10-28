#include"ShaderProgamMgr.h"
#include"Util.h"
#include<iostream>




std::weak_ptr<ShaderProgram> ShaderProgramManager::addProgram(const std::string& fileName, std::string name) {
	if (name.empty())
		name = ExtractFileNameFromPath(fileName, false);

	auto found = getProgram(name);
	if (!found.expired())
		return found;

	auto program = std::make_shared<ShaderProgram>(name, getResourcePath(fileName));
	if (program->compileAndLink()) {
		m_shaderPrograms.insert(std::make_pair(name, program));
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


std::string ShaderProgramManager::getResourcePath(const std::string& fileName) const {
	auto res = FileSystem::Default.getHomeDirectory();
	res /= "res/shader";
	res /= fileName;
	res = fs::canonical(res);

	if (!res.has_extension())
		res.concat(".shader");
	
	if (!fs::exists(res)) {
		std::cerr << "Walling: file \"" << res.string() << "\" not exist!" << std::endl;
#ifdef _DEBUG
		ASSERT(false);
#endif // DEBUG
	}

	return res.string();
}
