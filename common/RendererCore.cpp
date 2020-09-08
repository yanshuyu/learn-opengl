#include"RendererCore.h"


RenderContext::RenderContext(Renderer* renderer) :m_renderer(renderer)
, m_transformStack() {

}


void RenderContext::pushMatrix(const glm::mat4& m) {
	if (!m_transformStack.empty()) {
		m_transformStack.push(m_transformStack.top() * m);
		return;
	}
	m_transformStack.push(m);
}


void RenderContext::popMatrix() {
	m_transformStack.pop();
}


glm::mat4 RenderContext::getMatrix() const {
	return m_transformStack.top();
}