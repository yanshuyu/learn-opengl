#include"GLWindowApp.h"
#include<iostream>
#include<sstream>;

using std::cout;
using std::endl;

GLWindowApp::GLWindowApp(const std::string& name) :GLApplication(name) {

}


bool GLWindowApp::initailize() {
	bool success = __super::initailize();
	if (success) {
		std::stringstream ss;
		ss << "GL Info:\n" << "\tVendor: " << glGetString(GL_VENDOR) << endl;
		ss << "\tVersion: " << glGetString(GL_VERSION) << endl;
		ss << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
		ss << "\tShadingLang: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
		cout << ss.str();
	}

	return success;
}


void GLWindowApp::render() {
	glClearDepth(1.0);
	glClearColor(0.2, 0.4, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

