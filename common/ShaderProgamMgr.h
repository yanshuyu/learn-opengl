#pragma once
#include"ShaderProgram.h"
#include"Singleton.h"
#include"FileSystem.h"
#include<unordered_map>
#include<string>
#include<memory>




class ShaderProgramManager: public Singleton<ShaderProgramManager> {
public:
	ShaderProgramManager() {};

	std::weak_ptr<ShaderProgram> addProgram(const std::string& fileName, std::string name = "");
	std::weak_ptr<ShaderProgram> getProgram(const std::string name) const;
	bool removeProgram(const std::string& name);
	bool hasProgram(const std::string& name) const;
	
	inline void removeAllProgram() {
		m_shaderPrograms.clear();
	}

	inline size_t programCount() const {
		return m_shaderPrograms.size();
	}

protected:
	std::string getResourcePath(const std::string& fileName) const;

private:
	std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> m_shaderPrograms;
};