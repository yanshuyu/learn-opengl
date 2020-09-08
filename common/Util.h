#pragma once
#include<cassert>
#include<iostream>


#define ASSERT(exp) assert(exp);

#define GLCALL(exp) GLClearError();  \
	exp;  \
	ASSERT(GLCheckError())


void GLClearError();

bool GLCheckError();


typedef unsigned long ID;


