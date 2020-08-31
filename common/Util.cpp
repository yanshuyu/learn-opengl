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