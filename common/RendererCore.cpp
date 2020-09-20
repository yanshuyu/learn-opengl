#include"RendererCore.h"
#include"VertexArray.h"
#include"Mesh.h"
#include"Material.h"
#include<glm/gtc/matrix_transform.hpp>

Viewport_t::Viewport_t() : Viewport_t(0, 0, 1, 1) {

}


Viewport_t::Viewport_t(float _x, float _y, float _w, float _h) : x(_x)
, y(_y)
, width(_w)
, height(_h) {

}


Camera_t Camera_t::createDefault(float vpWidth, float vpHeight) {
	Camera_t defCam;
	defCam.near = 0.1f;
	defCam.far = 1000.f;
	defCam.backgrounColor = glm::vec4(glm::vec3(0.f), 1.0f);
	defCam.position = glm::vec3(0.f, 0.f, 0.f);
	defCam.viewport.x = 0;
	defCam.viewport.y = 0;
	defCam.viewport.width = vpWidth;
	defCam.viewport.height = vpHeight;
	defCam.viewMatrix = glm::lookAt(defCam.position,
											glm::vec3(0.f, 0.f, -1.f),
											glm::vec3(0.f, 1.f, 0.f));
	defCam.projMatrix = glm::perspective(glm::radians(60.f), vpWidth / vpHeight, defCam.near, defCam.far);
	
	return defCam;
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