#pragma once
#include<cassert>
#include<iostream>
#include<string>
#include<cstdint>
#include<glm/matrix.hpp>

using std::int8_t;
using std::uint8_t;
using std::int16_t;
using std::uint16_t;
using std::int32_t;
using std::uint32_t;
using std::int64_t;
using std::uint64_t;

using INT8 = int8_t;
using UINT8 = uint8_t;
using INT16 = int16_t;
using UINT16 = uint16_t;
using INT32 = int32_t;
using UINT32 = uint32_t;
using INT64 = int64_t;
using UINT64 = uint64_t;


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

std::string GLErrorStr(int error);

void ConsoleLog(const char* file, const char* func, int line, const std::string& msg);


//
// Helper
//
typedef unsigned long ID;

std::string ExtractFileNameFromPath(const std::string& path, bool includeExt = true);



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
