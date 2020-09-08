#include"ModelLoadingApp.h"
#include<common/Mesh.h>

ModelLoadingApp::ModelLoadingApp(const std::string& t, int w, int h) :GLApplication(t, w, h) {

}

bool ModelLoadingApp::initailize() {
	if (!__super::initailize())
		return false;

	//MeshManager::getInstance()->load("C:/Users/SY/Documents/learn-opengl/res/models/Alien_Animal.fbx");

	//Vertex_t vertices[3] = {};
	//auto sz = sizeof(Vertex_t);
	//auto stride = reinterpret_cast<int>(&vertices[1]) - reinterpret_cast<int>(&vertices[0]);
	
	m_renderer = std::make_unique<Renderer>();
	m_scene = std::make_unique<Scene>("model_loading_demo_scene", m_renderer.get());
}


void ModelLoadingApp::update(double dt) {

}


void ModelLoadingApp::render() {
	GLCALL(glClearColor(0, 1, 0, 1));
	GLCALL(glClearDepth(1.0));
	GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))

}