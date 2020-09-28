#include"RendererCore.h"
#include"VertexArray.h"
#include"Mesh.h"
#include"Material.h"
#include<glm/gtc/matrix_transform.hpp>
#include<vector>
#include<algorithm>


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



Light_t::Light_t(): type(LightType::Unknown)
, direction(0.f)
, color(0.f)
, position(0.f)
, range(0.f)
, innerCone(0.f)
, outterCone(0.f)
, intensity(0.f)
, shadowCamera()
, shadowType(ShadowType::NoShadow) 
, shadowBias(0.f)
, shadowStrength(0.f) {

}


bool Light_t::isCastShadow() const {
	return shadowType != ShadowType::NoShadow;
}


RenderingSettings_t::RenderingSettings_t(): renderSize(0.f)
, shadowMapResolution(0.f) {

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


ViewFrustum_t::ViewFrustum_t() :ltn(0.f)
, lbn(0.f)
, rtn(0.f)
, rbn(0.f)
, ltf(0.f)
, lbf(0.f)
, rtf(0.f)
, rbf(0.f) {

}

void ViewFrustum_t::applyTransform(const glm::mat4& t) {
	ltn = t * glm::vec4(ltn, 1.f);
	lbn = t * glm::vec4(lbn, 1.f);
	rtn = t * glm::vec4(rtn, 1.f);
	rbn = t * glm::vec4(rbn, 1.f);
	
	ltf = t * glm::vec4(ltf, 1.f);
	lbf = t * glm::vec4(lbf, 1.f);
	rtf = t * glm::vec4(rtf, 1.f);
	rbf = t * glm::vec4(rbf, 1.f);
}

ViewFrustum_t ViewFrustum_t::transform(const glm::mat4& t) const {
	ViewFrustum_t vf;
	vf.ltn = t * glm::vec4(ltn, 1.f);
	vf.lbn = t * glm::vec4(lbn, 1.f);
	vf.rtn = t * glm::vec4(rtn, 1.f);
	vf.rbn = t * glm::vec4(rbn, 1.f);

	vf.ltf = t * glm::vec4(ltf, 1.f);
	vf.lbf = t * glm::vec4(lbf, 1.f);
	vf.rtf = t * glm::vec4(rtf, 1.f);
	vf.rbf = t * glm::vec4(rbf, 1.f);

	return vf;
}

AABB_t ViewFrustum_t::getAABB() const {
	AABB_t aabb;
	std::vector<glm::vec3> points = { ltn, lbn, rtn, rbn, ltf, lbf, rtf, rbf };

	std::sort(points.begin(), points.end(), [](const glm::vec3& l, const glm::vec3& r) {
		return l.x < r.x;
	});
	aabb.minimum.x = points.front().x;
	aabb.maximum.x = points.back().x;

	std::sort(points.begin(), points.end(), [](const glm::vec3& l, const glm::vec3& r) {
		return l.y < r.y;
		});
	aabb.minimum.y = points.front().y;
	aabb.maximum.y = points.back().y;

	std::sort(points.begin(), points.end(), [](const glm::vec3& l, const glm::vec3& r) {
		return l.z < r.z;
		});
	aabb.minimum.z = points.front().z;
	aabb.maximum.z = points.back().z;

	return aabb;
}


float ViewFrustum_t::projectionLengthOnDirection(const glm::vec3& dir) const {
	glm::vec3 dirNorm = glm::normalize(dir);
	std::vector<glm::vec3> points = { ltn, lbn, rtn, rbn, ltf, lbf, rtf, rbf };
	std::vector<float> distancesOnDir;
	distancesOnDir.reserve(points.size());
	
	std::for_each(points.begin(), points.end(), [&](const glm::vec3& p) {
		distancesOnDir.push_back(fabs(glm::dot(p, dirNorm)));
	});
	std::sort(distancesOnDir.begin(), distancesOnDir.end());
	
	return distancesOnDir.back() - distancesOnDir.front();
}


AABB_t::AABB_t() : minimum(0.f),
maximum(0.f) {

}


ViewFrustum_t AABB_t::getFrustum() const {
#ifdef _DEBUG
	ASSERT(maximum.x >= minimum.x && maximum.y >= minimum.y && maximum.z >= minimum.z);
#endif // _DEBUG

	float w = maximum.x - minimum.x;
	float h = maximum.y - minimum.y;

	ViewFrustum_t vf;
	vf.lbn = minimum;
	vf.ltn = { minimum.x, minimum.y + h, minimum.z };
	vf.rbn = { minimum.x + w, minimum.y, minimum.z };
	vf.rtn = { minimum.x + w, minimum.y + h, minimum.z };

	vf.rtf = maximum;
	vf.rbf = { maximum.x, maximum.y - h, maximum.z };
	vf.ltf = { maximum.x - w, maximum.y, maximum.z };
	vf.lbf = { maximum.x - w, maximum.y - h, maximum.z };

	return vf;
}