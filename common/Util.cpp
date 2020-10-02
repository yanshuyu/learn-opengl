#include"Util.h"
#include<glad/glad.h>
#include<cstdarg>
#include<iostream>



void GLClearError() {
	while (glGetError());
}

bool GLCheckError() {
	bool hasError = false;
	while (GLenum e = glGetError()) {
		std::cerr << "glError occur: " << e << std::endl;
		hasError = true;
	}

	return !hasError;
}

std::string ExtractFileNameFromPath(const std::string& path, bool includeExt) {
	auto fst = path.find_last_of("\\");
	if (fst == std::string::npos)
		fst = path.find_last_of("/");
	
	fst = fst == std::string::npos ? 0 : fst + 1;

	auto last = path.length();
	if (!includeExt) {
		auto dot_pos = path.find_last_of(".");
		last = dot_pos == std::string::npos ? last : dot_pos;
	}

	return path.substr(fst, last - fst);
}

void ConsoleLog(const char* file, const char* func, int line, const std::string& msg){
	printf("%s @Function: %s @Line: %i: log: %s\n", file, func, line, msg.c_str());
	fflush(stdout);
}


std::string toStr(PrimitiveType pt) {
	switch (pt)
	{
	case PrimitiveType::Point:
		return "Point";

	case PrimitiveType::Line:
		return "Line";

	case PrimitiveType::Triangle:
		return "Triangle";

	case PrimitiveType::Polygon:
		return "Polygon";

	default:
		return "Unknown";
	}
}

std::string toStr(Shader::Type shaderType) {
	switch (shaderType)
	{
	case Shader::Type::VertexShader:
		return "VertexShader";
		
	case Shader::Type::GeometryShader:
		return "GeometryShader";

	case Shader::Type::FragmentShader:
		return "FragmentShader";

	default:
		return "Unknown";
	}
}