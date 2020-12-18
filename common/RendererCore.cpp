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
, uv(0.f) {

}


SkinVertex_t::SkinVertex_t() : joints(0)
, weights(0.f)
, position(0.f)
, normal(0.f)
, tangent(0.f)
, uv(0.f) {

}


Viewport_t::Viewport_t() : Viewport_t(0, 0, 1, 1) {

}


Viewport_t::Viewport_t(float _x, float _y, float _w, float _h) : x(_x)
, y(_y)
, width(_w)
, height(_h) {

}


ViewFrustum_t::ViewFrustum_t(): points() {
}

void ViewFrustum_t::applyTransform(const glm::mat4& t) {
	for (size_t i = 0; i < PointIndex::END; i++) {
		points[i] = t * glm::vec4(points[i], 1.f);
	}
}

ViewFrustum_t ViewFrustum_t::transform(const glm::mat4& t) const {
	ViewFrustum_t vf;
	for (size_t i = 0; i < PointIndex::END; i++) {
		vf.points[i] = t * glm::vec4(points[i], 1.f);
	}
	return vf;
}

std::vector<ViewFrustum_t> ViewFrustum_t::split(std::vector<float> percentages) const {
	std::vector<ViewFrustum_t> subFrustums;
	float lastPercent = 0.f;
	percentages.push_back(1.f);
	for (float percent : percentages) {
		ViewFrustum_t vf;
		
		vf.points[LBN] = points[LBN] + (points[LBF] - points[LBN]) * lastPercent;
		vf.points[LTN] = points[LTN] + (points[LTF] - points[LTN]) * lastPercent;
		vf.points[RTN] = points[RTN] + (points[RTF] - points[RTN]) * lastPercent;
		vf.points[RBN] = points[RBN] + (points[RBF] - points[RBN]) * lastPercent;

		vf.points[LBF] = points[LBN] + (points[LBF] - points[LBN]) * percent;
		vf.points[LTF] = points[LTN] + (points[LTF] - points[LTN]) * percent;
		vf.points[RTF] = points[RTN] + (points[RTF] - points[RTN]) * percent;
		vf.points[RBF] = points[RBN] + (points[RBF] - points[RBN]) * percent;

		subFrustums.push_back(vf);
		lastPercent = percent;
	}

	return std::move(subFrustums);
}

AABB_t ViewFrustum_t::getAABB() const {
	AABB_t aabb;
	
	std::array<glm::vec3, PointIndex::END> copyPoints;
	std::copy(points.begin(), points.end(), copyPoints.begin());

	std::sort(copyPoints.begin(), copyPoints.end(), [](const glm::vec3& l, const glm::vec3& r) {
		return l.x < r.x;
		});
	aabb.minimum.x = copyPoints.front().x;
	aabb.maximum.x = copyPoints.back().x;

	std::sort(copyPoints.begin(), copyPoints.end(), [](const glm::vec3& l, const glm::vec3& r) {
		return l.y < r.y;
		});
	aabb.minimum.y = copyPoints.front().y;
	aabb.maximum.y = copyPoints.back().y;

	std::sort(copyPoints.begin(), copyPoints.end(), [](const glm::vec3& l, const glm::vec3& r) {
		return l.z < r.z;
		});
	aabb.minimum.z = copyPoints.front().z;
	aabb.maximum.z = copyPoints.back().z;

	return aabb;
}

glm::vec3 ViewFrustum_t::getCenter() const {
	glm::vec3 sum(0.f);
	std::for_each(points.begin(), points.end(), [&](const glm::vec3& p) {
		sum += p;
	});

	return sum / float(points.size());
}

std::ostream& operator << (std::ostream& o, const ViewFrustum_t& vf) {
	o << "[ ltn(" << vf.points[ViewFrustum_t::PointIndex::LTN].x << ", " << vf.points[ViewFrustum_t::PointIndex::LTN].y << ", " << vf.points[ViewFrustum_t::PointIndex::LTN].z << ")\t";
	o << " rtn(" << vf.points[ViewFrustum_t::PointIndex::RTN].x << ", " << vf.points[ViewFrustum_t::PointIndex::RTN].y << ", " << vf.points[ViewFrustum_t::PointIndex::RTN].z << ")\n";
	o << " lbn(" << vf.points[ViewFrustum_t::PointIndex::LBN].x << ", " << vf.points[ViewFrustum_t::PointIndex::LBN].y << ", " << vf.points[ViewFrustum_t::PointIndex::LBN].z << ")\t";
	o << " rtn(" << vf.points[ViewFrustum_t::PointIndex::RBN].x << ", " << vf.points[ViewFrustum_t::PointIndex::RBN].y << ", " << vf.points[ViewFrustum_t::PointIndex::RBN].z << ") ]\n";

	o << "[ ltf(" << vf.points[ViewFrustum_t::PointIndex::LTF].x << ", " << vf.points[ViewFrustum_t::PointIndex::LTF].y << ", " << vf.points[ViewFrustum_t::PointIndex::LTF].z << ")\t";
	o << " rtf(" << vf.points[ViewFrustum_t::PointIndex::RTF].x << ", " << vf.points[ViewFrustum_t::PointIndex::RTF].y << ", " << vf.points[ViewFrustum_t::PointIndex::RTF].z << ")\n";
	o << " lbf(" << vf.points[ViewFrustum_t::PointIndex::LBF].x << ", " << vf.points[ViewFrustum_t::PointIndex::LBF].y << ", " << vf.points[ViewFrustum_t::PointIndex::LBF].z << ")\t";
	o << " rtf(" << vf.points[ViewFrustum_t::PointIndex::RBF].x << ", " << vf.points[ViewFrustum_t::PointIndex::RBF].y << ", " << vf.points[ViewFrustum_t::PointIndex::RBF].z << ") ]\n";
	return o;
}

AABB_t::AABB_t() : minimum(0.f),
maximum(0.f) {

}

Camera_t::Camera_t(): viewMatrix(1.f)
, projMatrix(1.f)
, backgrounColor(0.f)
, position(0.f)
, lookDirection(0.f)
, near(0.f)
, far(0.f) 
, fov(0.f)
, aspectRatio(1.f)
, viewport() 
, viewFrustum() {

}


Light_t::Light_t(): type(LightType::Unknown)
, direction(0.f)
, color(0.f)
, position(0.f)
, range(0.f)
, innerCone(0.f)
, outterCone(0.f)
, intensity(0.f)
, shadowType(ShadowType::NoShadow) 
, shadowBias(0.f)
, shadowStrength(0.f) {

}


bool Light_t::isCastShadow() const {
	return shadowType != ShadowType::NoShadow;
}


MeshRenderItem_t::MeshRenderItem_t(): vao(nullptr)
, material(nullptr)
, bonesTransform(nullptr)
, indexCount(0)
, vertexCount(0)
, boneCount(0)
, primitive(PrimitiveType::Unknown)
, modelMatrix(1.f) {
}


Scene_t::Scene_t() {
	reset();
}

void Scene_t::clear() {
	reset();
}

void Scene_t::reset() {
	opaqueItems = nullptr;
	numOpaqueItems = 0;

	cutOutItems = nullptr;
	numCutOutItems = 0;
	
	transparentItems = nullptr;
	numTransparentItems = 0;
		
	lights = nullptr;
	numLights = 0;
	
	assistCameras = nullptr;
	numAssistCameras = 0;

	mainCamera = nullptr;

	skyBox = nullptr;

	ambinetSky = glm::vec3(0);
	ambinetGround = glm::vec3(0);
}


void RenderContext::pushMatrix(const glm::mat4& m) {
	if (!m_transforms.empty()) {
		m_transforms.push(m_transforms.top() * m);
		return;
	}
	m_transforms.push(m);
}


void RenderContext::clearMatrix() {
	while (!m_transforms.empty())
		m_transforms.pop();
}


GPUPipelineState::GPUPipelineState() :cullMode(CullFaceMode::Back)
, cullFaceWindingOrder(FaceWindingOrder::CCW)
, shadeMode(ShadeMode::Smooth)
, fillMode(FillMode::Fill)
, depthMode(DepthMode::Disable)
, depthFunc(DepthFunc::Less)
, depthMask(1)
, stencilMode(StencilMode::Disable)
, stencilPassOp(StencilOp::Zero)
, stencilFailOp(StencilOp::Zero)
, stencilDepthFailOp(StencilOp::Zero)
, stencilMask(0xffffffff)
, blendMode(BlendMode::Disable)
, blendSrcFactor(BlendFactor::SrcAlpha)
, blendDstFactor(BlendFactor::One_Minus_Src_Alpha)
, blendFunc(BlendFunc::Add)
, blendColor{ 1.f, 1.f, 1.f, 1.f } {

}


GPUPipelineState GPUPipelineState::s_defaultState;



std::string PrimitiveType2Str(PrimitiveType pt) {
	switch (pt)
	{
	case PrimitiveType::Point:
		return "Point";

	case PrimitiveType::Line:
		return "Line";

	case PrimitiveType::Triangle:
		return "Triangle";

	case PrimitiveType::Polygon:
		return "Polygon";

	default:
		return "Unknown";
	}
}
