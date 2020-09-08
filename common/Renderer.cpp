#include"Renderer.h"
#include"Util.h"

Renderer::Renderer() :m_clearColor{ 0 }
, m_clearDepth(1.0f)
, m_clearMask(0){

}

void Renderer::setViewPort(int x, int y, int width, int height) {
	GLCALL(glViewport(x, y, width, height));
}


void Renderer::setClearColor(float r, float g, float b, float a) {
	m_clearColor[0] = r;
	m_clearColor[1] = g;
	m_clearColor[2] = b;
	m_clearColor[3] = a;
	GLCALL(glClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]));
}


void Renderer::setClearDepth(float d) {
	m_clearDepth = d;
	GLCALL(glClearDepth(m_clearDepth));
}


void Renderer::setClearMask(int m) {
	m_clearMask = m;
	GLCALL(glClearStencil(m_clearMask));
}


void Renderer::clearScrren(int flags) {
	GLCALL(glClear(flags));
}

void Renderer::subsimtTask(const RenderTask_t& task) {

}

void Renderer::beginDepthPass() {
	GLCALL(glEnable(GL_DEPTH_TEST));
	GLCALL(glDepthFunc(GL_LESS));
	GLCALL(glDepthMask(GL_TRUE));
}

void Renderer::endDepthPass() {

}