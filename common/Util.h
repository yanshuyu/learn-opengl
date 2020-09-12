#pragma once
#include<cassert>
#include<iostream>
#include<string>


#define ASSERT(exp) assert(exp);

#define GLCALL(exp) GLClearError();  \
	exp;  \
	ASSERT(GLCheckError())


void GLClearError();

bool GLCheckError();


typedef unsigned long ID;



bool ExtractFileNameFromPath(const std::string& path, std::string& name, bool includeExt = true);

