#include"RenderTechnique.h"
#include"Renderer.h"
#include"Util.h"
#include<glad/glad.h>


RenderTechnique::RenderTechnique(Renderer* invoker): m_invoker(invoker)
, m_clearColor{0.f, 0.f, 0.f, 1.f}
, m_clearDepth(1.0f)
, m_clearStencil(0)
, m_viewPort(){
	setClearColor(m_clearColor);
	setClearDepth(m_clearDepth);
	setClearStencil(m_clearStencil);
}


void RenderTechnique::setViewPort(const Viewport_t& vp) {
	m_viewPort = vp;
	GLCALL(glViewport(m_viewPort.x, m_viewPort.y , m_viewPort.width, m_viewPort.height));
}


void RenderTechnique::setClearColor(const glm::vec4& color) {
	m_clearColor = color;
	GLCALL(glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a));
}


void RenderTechnique::setClearDepth(float d) {
	m_clearDepth = d;
	GLCALL(glClearDepth(m_clearDepth));
}


void RenderTechnique::setClearStencil(int m) {
	m_clearStencil = m;
	GLCALL(glClearStencil(m_clearStencil));
}


void RenderTechnique::pullingRenderTask(ShaderProgram* shader) {
	m_invoker->pullingRenderTask();
}
