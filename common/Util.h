#pragma once
#include<cassert>
#include<iostream>
#include<string>
#include"Shader.h"
#include"RendererCore.h"



#define ASSERT(exp) assert(exp);

#define GLCALL(exp) GLClearError();  \
	exp;  \
	ASSERT(GLCheckError())

#define CONSOLELOG(msg) ConsoleLog(__FILE__, __FUNCTION__, __LINE__, msg)

void GLClearError();

bool GLCheckError();


typedef unsigned long ID;

void ConsoleLog(const char* file, const char* func, int line, const std::string& msg);

std::string ExtractFileNameFromPath(const std::string& path, bool includeExt = true);


std::string toStr(PrimitiveType pt);
std::string toStr(Shader::Type shaderType);