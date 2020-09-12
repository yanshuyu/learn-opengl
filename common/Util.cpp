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

bool ExtractFileNameFromPath(const std::string& path, std::string& name, bool includeExt) {
	auto fst = path.find_last_of("\\");
	if (fst == std::string::npos)
		fst = path.find_last_of("/");
	if (fst == std::string::npos)
		return false;

	auto last = path.length();
	if (!includeExt)
		last = path.find_last_of(".");
	if (last == std::string::npos)
		return false;

	name = path.substr(++fst, last - fst);
	
	return true;
}