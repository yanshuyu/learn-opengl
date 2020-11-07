#pragma once
#include<cassert>
#include<iostream>
#include<string>
#include"Shader.h"
#include"RendererCore.h"


//
// Debug
//
#define ASSERT(exp) assert(exp);

#define GLCALL(exp) GLClearError();  \
	exp;  \
	ASSERT(GLCheckError())

#define CONSOLELOG(msg) ConsoleLog(__FILE__, __FUNCTION__, __LINE__, msg)

void GLClearError();

bool GLCheckError();

void ConsoleLog(const char* file, const char* func, int line, const std::string& msg);


//
// Helper
//
typedef unsigned long ID;

std::string ExtractFileNameFromPath(const std::string& path, bool includeExt = true);

std::string toStr(PrimitiveType pt);
std::string toStr(Shader::Type shaderType);


//
// Math
//
#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)
#define FLT_CMP(x, y)  (fabsf(x - y) <= FLT_EPSILON * fmaxf(1.0f, fmaxf(fabsf(x), fabsf(y))))

template<typename T> T clamp(T x, T min, T max) {
	x = MAX(x, min);
	x = MIN(x, max);
	
	return x;
}


//
// Liner Algebra
//
bool IsOrthoNormal(const glm::mat3& m);
void OrthoNormalize(glm::mat3& m);
