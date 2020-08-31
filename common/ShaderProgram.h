#pragma once
#include<glad/glad.h>
#include"Util.h"
#include<vector>
#include<iostream>


class ShaderProgramManager;

class ShaderProgram {
	friend class ShaderProgramManager;

public:
	struct Attribute {
		Attribute() {
			elemenTtype = 0;
			name = "";
			location = -1;
			elementSize = 0;
		}

		GLenum elemenTtype;
		std::string name;
		int location;
		int elementSize;
	};

	typedef Attribute Uniform;

	struct UniformBlock {
		std::string name;
		int index;
		int dataSize;
		std::vector<int> uniformIndices;
	};

public:
	ShaderProgram(const std::string& name, const std::string& file);
	~ShaderProgram();

	bool compileAndLink();
	bool isLinked() const;
	void release();
	
	GLuint getHandler() const;
	std::string getInfoLog() const;
	void dumpProgramInfo() const;

	std::vector<Attribute> getAttributes() const;
	std::vector<Uniform> getUniforms() const;
	std::vector<UniformBlock> getUniformBlocks() const;

	void bind() const;
	void unbind() const;

	template<typename T>
	bool setUniform1(const std::string& name, T t1) const;;
	
	template<typename T>
	bool setUniform2(const std::string& name, T t1, T t2) const;
	
	template<typename T>
	bool setUniform3(const std::string& name, T t1, T t2, T t3) const;

	template<typename T>
	bool setUniform4(const std::string& name, T t1, T t2, T t3, T t4) const;

	template<typename T>
	bool setUniform1v(const std::string& name, T* data, size_t count = 1) const;

	template<typename T>
	bool setUniform2v(const std::string& name, T* data, size_t count = 1) const;

	template<typename T>
	bool setUniform3v(const std::string& name, T* data, size_t count = 1) const;

	template<typename T>
	bool setUniform4v(const std::string& name, T* data, size_t count = 1) const;

	template<typename T>
	bool setUniformMat4v(const std::string& name, T* data, size_t count = 1) const;


	
protected:
	bool parseShaderSource(std::string& vs, std::string& fs);
	void queryProgramInfo();
	int getUniformLocation(const std::string& name) const;

protected:
	GLuint m_handler;
	bool m_linked;
	std::string m_name;
	std::string m_file;

	std::vector<Attribute> m_attributes;
	std::vector<Uniform> m_uniforms;
	std::vector<UniformBlock> m_uniformBlocks;

};


std::ostream& operator << (std::ostream& o, const ShaderProgram::Attribute& a);
std::ostream& operator << (std::ostream& o, const ShaderProgram::UniformBlock& ub);


