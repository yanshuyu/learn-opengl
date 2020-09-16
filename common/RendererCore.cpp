#include"RendererCore.h"


Viewport_t::Viewport_t() : Viewport_t(0, 0, 1, 1) {

}


Viewport_t::Viewport_t(float _x, float _y, float _w, float _h) : x(_x)
, y(_y)
, width(_w)
, height(_h) {

}


RenderContext::RenderContext(Renderer* renderer) :m_renderer(renderer)
, m_transformStack() {

}

bool operator == (const Viewport_t& lhs, const Viewport_t& rhs) {
	return lhs.x == rhs.x
		&& lhs.y == rhs.y
		&& lhs.width == rhs.width
		&& lhs.height == rhs.height;
}

bool operator != (const Viewport_t& lhs, const Viewport_t& rhs) {
	return !(lhs == rhs);
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


void RenderContext::clearMatrix() {
	while (!m_transformStack.empty()) {
		m_transformStack.pop();
	}
}

glm::mat4 RenderContext::getMatrix() const {
	return m_transformStack.top();
}