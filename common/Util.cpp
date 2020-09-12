#include"Util.h"
#include<glad/glad.h>
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

bool ExtractFileNameFromPath(const std::string& path, std::string& name) {
	auto pos = path.find_last_of("\\");
	if (pos == std::string::npos)
		pos = path.find_last_of("/");
	if (pos == std::string::npos)
		return false;

	name = path.substr(++pos);
	
	return true;
}