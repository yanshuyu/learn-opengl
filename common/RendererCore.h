#pragma once
#include<glm/glm.hpp>
#include<stack>
#include<vector>


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


struct Camera_t {
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
	glm::vec4 backgrounColor;
	glm::vec3 position;
	Viewport_t viewport;
	float near;
	float far;

	static Camera_t createDefault(float vpWidth, float vpHeight);

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

	Camera_t shadowCamera;
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




