#include"RendererCore.h"
#include"VertexArray.h"
#include"Mesh.h"
#include"Material.h"
#include<glm/gtc/matrix_transform.hpp>


Vertex_t::Vertex_t(): position(0.f)
, normal(0.f)
, tangent(0.f)
, biTangent(0.f)
, uv(0.f) {

}


Viewport_t::Viewport_t() : Viewport_t(0, 0, 1, 1) {

}


Viewport_t::Viewport_t(float _x, float _y, float _w, float _h) : x(_x)
, y(_y)
, width(_w)
, height(_h) {

}


Camera_t::Camera_t(): viewMatrix(1.f)
, projMatrix(1.f)
, backgrounColor(0.f)
, position(0.f)
, viewport()
, near(0.f)
, far(0.f) {

}


Camera_t Camera_t::createDefault(float vpWidth, float vpHeight) {
	Camera_t defCam;

	if (vpWidth <= 0 || vpHeight <= 0)
		return defCam;

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


Light_t::Light_t(): type(LightType::Unknown)
, direction(0.f)
, color(0.f)
, position(0.f)
, range(0.f)
, innerCone(0.f)
, outterCone(0.f)
, intensity(0.f) {

}


RenderTask_t::RenderTask_t(): vao(nullptr)
, material(nullptr)
, indexCount(0)
, vertexCount(0)
, primitive(PrimitiveType::Unknown)
, modelMatrix(1.f) {
}


SceneRenderInfo_t::SceneRenderInfo_t(): camera()
, lights() {

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