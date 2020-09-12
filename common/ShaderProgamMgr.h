#pragma once
#include"ShaderProgram.h"
#include"Singleton.h"
#include<unordered_map>
#include<string>
#include<memory>




class ShaderProgramManager: public Singleton<ShaderProgramManager> {
public:
	ShaderProgramManager() {};

	std::shared_ptr<ShaderProgram> addProgram(const std::string& name, const std::string& file);
	std::shared_ptr<ShaderProgram> getProgram(const std::string name) const;
	std::shared_ptr<ShaderProgram> removeProgram(const std::string& name);
	bool hasProgram(const std::string& name) const;
	
	inline void removeAllProgram() {
		m_shaderPrograms.clear();
	}

	inline size_t programCount() const {
		return m_shaderPrograms.size();
	}

	inline std::string getResourceAbsolutePath() const {
		return "C:/Users/SY/Documents/learn-opengl/res/shader/";
	}

	inline std::string getResourceRelativePath() const {
		return "res/shader/";
	}

private:
	std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> m_shaderPrograms;
};