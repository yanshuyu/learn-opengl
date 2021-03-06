#include"Util.h"
#include"RendererCore.h"
#include<glad/glad.h>
#include<cstdarg>
#include<iostream>



void GLClearError() {
	while (glGetError());
}

bool GLCheckError() {
	bool hasError = false;
	while (GLenum e = glGetError()) {
		std::cerr << "glError occur: (" << e << ")" << GLErrorStr(e) <<std::endl;
		hasError = true;
	}

	return !hasError;
}


std::string GLErrorStr(int error) {
	switch (error)
	{
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM, enumeration parameter is not legal.";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE, value parameter is not legal.";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION,  state for a command is not legal for its given parameters.";
	case GL_STACK_OVERFLOW:
		return "GL_STACK_OVERFLOW, stack pushing operation causes a stack overflow.";
	case GL_STACK_UNDERFLOW:
		return	"GL_STACK_UNDERFLOW, stack popping operation occurs while the stack is at its lowest point.";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY, memory allocation operation cannot allocate (enough) memory.";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "GL_INVALID_FRAMEBUFFER_OPERATION, reading or writing to a framebuffer that is not complete.";
	default:
		break;
	}

	return "Unknow error.";
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


bool IsOrthoNormal(const glm::mat3& m) {
	if (!FLT_CMP(glm::dot(m[0], m[0]), 1.f) || !FLT_CMP(glm::dot(m[1], m[1]), 1.f) || !FLT_CMP(glm::dot(m[2], m[2]), 1.f))
		return false;
	if (!FLT_CMP(glm::dot(m[0], m[1]), 0.f) || !FLT_CMP(glm::dot(m[0], m[2]), 0.f) || !FLT_CMP(glm::dot(m[1], m[2]), 0.f))
		return false;

	return true;
}


void OrthoNormalize(glm::mat3& m) {
	auto x = m[0];
	auto y = m[1];
	auto z = m[2];
	x = glm::normalize(x);
	z = glm::normalize(glm::cross(x, y));
	y = glm::cross(z, x);
	
	m[0] = x;
	m[1] = y;
	m[2] = z;
}