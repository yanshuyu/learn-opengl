#pragma once
#include"Shader.h"
#include<glad/glad.h>
#include<vector>
#include<unordered_map>
#include<iostream>


class ShaderProgramManager;


class ShaderProgram {
	friend class ShaderProgramManager;
	friend std::ostream& operator << (std::ostream& o, const ShaderProgram& program);

public:
	struct Attribute {
		GLenum elemenTtype;
		std::string name;
		int location;
		int elementSize;

		Attribute() : elemenTtype(0)
			, name()
			, location(-1)
			, elementSize(0) {

		}
	};

	typedef Attribute Uniform;

	struct UniformBlock {
		std::string name;
		int index;
		int dataSize;
		std::vector<int> uniformIndices;
		
		UniformBlock() : name()
			, index(-1)
			, dataSize(0)
			, uniformIndices() {

		}
	};


	struct Subroutine {
		std::string name;
		int index;
		Shader::Type shaderStage;

		Subroutine() : name()
			, index(-1)
			, shaderStage(Shader::Type::Unknown) {

		}
	};

	
	struct SubroutineUniform {
		std::string name;
		int location;
		Shader::Type shaderStage;
		int size;
		std::vector<int> compatibleSubroutineIndices;

		SubroutineUniform() : name()
			, location(-1)
			, shaderStage(Shader::Type::Unknown)
			, size(0)
			, compatibleSubroutineIndices() {

		}
	};


	struct StageSubroutineInfo {
		Shader::Type stage;
		std::vector<Subroutine> subroutines;
		std::vector<SubroutineUniform> subroutineUniforms;

		StageSubroutineInfo(): stage(Shader::Type::Unknown)
			, subroutines()
			, subroutineUniforms() {
		}
	};

	enum class UniformBlockBindingPoint {
		Unknown,
		MaterialBlock,
		LightBlock,
		ShadowBlock,
	};

public:
	ShaderProgram(const std::string& name, const std::string& file);
	~ShaderProgram();

	ShaderProgram(const ShaderProgram& other) = delete;
	ShaderProgram& operator = (const ShaderProgram& other) = delete;

	bool compileAndLink();
	bool isLinked() const;
	void release();
	
	GLuint getHandler() const;
	std::string getInfoLog() const;

	bool hasAttribute(const std::string& name) const;
	bool hasUniform(const std::string& name) const;
	bool hasUniformBlock(const std::string& name) const;
	bool hasSubroutineUniform(Shader::Type shaderStage, const std::string& name) const;

	void bind() const;
	void unbind() const;
	bool isBinded() const;

	bool bindUniformBlock(const std::string& name, UniformBlockBindingPoint bp);
	void unbindUniformBlock(const std::string& name);

	bool setSubroutineUniforms(Shader::Type shaderStage, const std::unordered_map<std::string, std::string>& mapping);
	const std::vector<Subroutine>& getSubroutines(Shader::Type shaderStage) const;
	const std::vector<SubroutineUniform>& getSubroutineUniforms(Shader::Type shaderStage) const;

	template<typename T>
	bool setUniform1(const std::string& name, T t1) const;
	
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


	inline const std::vector<Attribute>& getAttributes() const {
		return m_attributes;
	}

	inline const std::vector<Uniform>& getUniforms() const {
		return m_uniforms;
	}

	inline const std::vector<UniformBlock>& getUniformBlocks() const {
		return m_uniformBlocks;
	}

protected:
	bool parseShaderSource(std::string& vs, std::string& gs, std::string& fs);
	void queryProgramInfo();
	int getUniformLocation(const std::string& name) const;
	int getUniformBlockIndex(const std::string& name) const;
	
	SubroutineUniform* getSubroutineUniform(Shader::Type shaderStage, const std::string& name) const;
	Subroutine* getSubroutine(Shader::Type shaderStage, const std::string& name) const;
	bool checkSubroutineCompatible(SubroutineUniform* su, Subroutine* st) const;

protected:
	GLuint m_handler;
	bool m_linked;
	std::string m_name;
	std::string m_file;

	std::vector<Attribute> m_attributes;
	std::vector<Uniform> m_uniforms;
	std::vector<UniformBlock> m_uniformBlocks;

	mutable std::unordered_map<Shader::Type, StageSubroutineInfo> m_stageSubroutinesInfo;
};


std::ostream& operator << (std::ostream& o, const ShaderProgram::Attribute& a);
std::ostream& operator << (std::ostream& o, const ShaderProgram::UniformBlock& ub);
std::ostream& operator << (std::ostream& o, const ShaderProgram::Subroutine& st);
std::ostream& operator << (std::ostream& o, const ShaderProgram::SubroutineUniform& su);
std::ostream& operator << (std::ostream& o, const ShaderProgram& program);


