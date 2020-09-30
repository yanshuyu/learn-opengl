#pragma once
#include<glm/glm.hpp>
#include<stack>
#include<vector>
#include<array>


class VertexArray;
class Material;
class Renderer;

typedef unsigned int Index_t;

struct Vertex_t {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 biTangent;
	glm::vec2 uv;

	Vertex_t();
};


enum class PrimitiveType {
	Point,
	Line,
	Triangle,
	Polygon,
	Unknown,
};


enum class LightType {
	Unknown,
	DirectioanalLight,
	PointLight,
	SpotLight,
};


enum class ShadowType {
	NoShadow,
	HardShadow,
	SoftShadow,
};


struct Viewport_t {
	float x;
	float y;
	float width;
	float height;

	Viewport_t();
	Viewport_t(float _x, float _y, float _w, float _h);
};

bool operator == (const Viewport_t& lhs, const Viewport_t& rhs);
bool operator != (const Viewport_t& lhs, const Viewport_t& rhs);


struct AABB_t;

struct ViewFrustum_t {
	enum PointIndex {
		LBN,
		LTN,
		RTN,
		RBN,

		LBF,
		LTF,
		RTF,
		RBF,
		
		END,
	};
	
	std::array<glm::vec3, PointIndex::END> points;

	ViewFrustum_t();
	void applyTransform(const glm::mat4& t);
	ViewFrustum_t transform(const glm::mat4& t) const;
	std::vector<ViewFrustum_t> split(std::vector<float> percentages) const; // split view frustum for cascade shadow map
	AABB_t getAABB() const;
	glm::vec3 getCenter() const;
};

std::ostream& operator << (std::ostream& o, const ViewFrustum_t& vf);

struct AABB_t {
	glm::vec3 minimum;
	glm::vec3 maximum;

	AABB_t();
};


struct Camera_t {
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
	glm::vec4 backgrounColor;
	glm::vec3 position;
	glm::vec3 lookDirection;
	float near;
	float far;
	float fov;
	float aspectRatio;
	Viewport_t viewport;
	ViewFrustum_t viewFrustum;

	Camera_t();
};


struct Light_t {
	LightType type;
	glm::vec3 direction;
	glm::vec3 color;
	glm::vec3 position;
	float range;
	float innerCone;
	float outterCone;
	float intensity;

	ShadowType shadowType;
	float shadowBias;
	float shadowStrength;

	bool isCastShadow() const;

	Light_t();
};


struct RenderingSettings_t {
	glm::vec2 renderSize;
	glm::vec2 shadowMapResolution;

	RenderingSettings_t();
};


struct RenderTask_t {
	const VertexArray* vao;
	const Material* material;
	size_t indexCount;
	size_t vertexCount;
	PrimitiveType primitive;
	glm::mat4 modelMatrix;

	RenderTask_t();
};



struct SceneRenderInfo_t {
	Camera_t camera;
	std::vector<Light_t> lights;
	// maybe will add enviroment setting, ect.

	SceneRenderInfo_t();
};



enum class RenderPass {
	None,
	DepthPass,
	GeometryPass,
	UnlitPass,
	ShadowPass,
	LightPass,
	TransparencyPass,
};



class RenderContext {
public:
	RenderContext(Renderer* renderer = nullptr);

	void pushMatrix(const glm::mat4& m);
	void popMatrix();
	void clearMatrix();
	glm::mat4 getMatrix() const;


	inline void setRenderer(Renderer* renderer) {
		m_renderer = renderer;
	}
	
	inline Renderer* renderer() const {
		return m_renderer;
	}

private:
	Renderer* m_renderer;
	std::stack<glm::mat4> m_transformStack;
};


//
// uniform block structs
//
struct DirectionalLightBlock {
	glm::vec4 color;
	glm::vec3 inverseDiretion;
};

struct PointLightBlock {
	glm::vec4 position;
	glm::vec4 color;
};

struct SpotLightBlock {
	glm::vec4 position;
	glm::vec4 color;
	glm::vec3 inverseDirection;
	glm::vec2 angles;
};

struct ShadowBlock {
	glm::mat4 lightVP;
	float shadowStrength;
	float depthBias;
	int shadowType;
};


template<size_t N>
struct CascadeShadowBlock {
	glm::mat4 lightVP[N];
	float cascadesFarZ[N];
	float shadowStrength;
	float shadowBias;
	int shadowType;
};

