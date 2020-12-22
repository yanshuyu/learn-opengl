#pragma once
#include<glm/glm.hpp>
#include<glad/glad.h>
#include"Containers.h"

class VertexArray;
class Texture;
class IMaterial;
class Renderer;

typedef unsigned int Index_t;

struct Vertex_t {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec2 uv;

	Vertex_t();
};

struct SkinVertex_t {
	glm::ivec4 joints;
	glm::vec4 weights;
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec2 uv;

	SkinVertex_t();
};


enum class PrimitiveType {
	Point,
	Line,
	Triangle,
	Polygon,
	Unknown,
};

std::string PrimitiveType2Str(PrimitiveType pt);


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
	Viewport_t(const Viewport_t& other) = default;
	Viewport_t& operator = (const Viewport_t& other) = default;

	inline bool operator == (const Viewport_t& other) {
		return  FLT_CMP(x, other.x) && FLT_CMP(y, other.y) && FLT_CMP(width, other.width) && FLT_CMP(height, other.height);
	}

	inline bool operator != (const Viewport_t& other) {
		return !(*this == other);
	}
};

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


struct SkyBox_t {
	VertexArray* cubeVAO;
	Texture* cubeMap;

	operator bool () const { return cubeVAO != nullptr && cubeMap != nullptr; }
	inline void reset() { cubeVAO = nullptr; cubeMap = nullptr; }

	SkyBox_t() : SkyBox_t(nullptr, nullptr) {}
	SkyBox_t(VertexArray* cv, Texture* cm) : cubeVAO(cv), cubeMap(cm) {}
};


struct MeshRenderItem_t {
	const VertexArray* vao;
	const IMaterial* material;
	const glm::mat4* bonesTransform;
	size_t indexCount;
	size_t vertexCount;
	size_t boneCount;

	PrimitiveType primitive;
	glm::mat4 modelMatrix;

	MeshRenderItem_t();
};


struct Scene_t {
	MeshRenderItem_t* opaqueItems;
	size_t numOpaqueItems;

	MeshRenderItem_t* cutOutItems;
	size_t numCutOutItems;
	
	MeshRenderItem_t* transparentItems;
	size_t numTransparentItems;
	
	Light_t* lights;
	size_t numLights;
	
	Camera_t* assistCameras;
	size_t numAssistCameras;

	Camera_t* mainCamera;

	SkyBox_t* skyBox;

	glm::vec3 ambinetSky;
	glm::vec3 ambinetGround;

	Scene_t();
	void clear();

private:
	void reset();
};


enum class RenderPass {
	None,
	DepthPass,
	GeometryPass,
	UnlitPass,
	ShadowPass,
	LightPass,
	AmbientPass,
	TransparencyPass,
};


class RenderContext {
public:
	RenderContext() = default;

	void pushMatrix(const glm::mat4& m);
	void clearMatrix();

	inline void popMatrix() {
		m_transforms.pop();
	}

	inline const glm::mat4& getMatrix() const {
		return m_transforms.top();
	}

	inline size_t matrixSize() const {
		return m_transforms.size();
	}

	inline Renderer* getRenderer() const {
		return m_renderer;
	}
	
	inline void setRenderer(Renderer* renderer) {
		m_renderer = renderer;
	}

protected:
	PoolStack<glm::mat4> m_transforms;
	Renderer* m_renderer;
};



enum ClearFlags {
	Color = GL_COLOR_BUFFER_BIT,
	Depth = GL_DEPTH_BUFFER_BIT,
	Stencil = GL_STENCIL_BUFFER_BIT,
};

enum class CullFaceMode {
	None,
	Front = GL_FRONT,
	Back = GL_BACK,
	Both = GL_FRONT_AND_BACK,
};

enum class FaceWindingOrder {
	CW = GL_CW,
	CCW = GL_CCW,
};

enum class ShadeMode {
	Smooth = GL_SMOOTH,
	Flat = GL_FLAT,
};


enum class FillMode {
	Fill = GL_FILL,
	Line = GL_LINE,
	Point = GL_POINT,
};


enum class DepthMode {
	Disable,
	Enable,
};

enum class StencilMode {
	Disable,
	Enable,
};

enum class  DepthFunc {
	Never = GL_NEVER,
	Less = GL_LESS,
	Equal = GL_EQUAL,
	LEqual = GL_LEQUAL,
	Greater = GL_GREATER,
	NotEqual = GL_NOTEQUAL,
	GEqual = GL_GEQUAL,
	Always = GL_ALWAYS,
};


typedef DepthFunc StencilFunc;


enum class StencilOp {
	Zero = GL_ZERO,
	Keep = GL_KEEP,
	Replace = GL_REPLACE,
	Increment = GL_INCR,
	IncrementWrap = GL_INCR_WRAP,
	Decrement = GL_DECR,
	DecrementWrap = GL_DECR_WRAP,
	Invert = GL_INVERT,
};



enum class BlendMode {
	Disable,
	Enable,
};

enum class BlendFactor {
	Zero = GL_ZERO,
	One = GL_ONE,
	SrcColor = GL_SRC_COLOR,
	One_Minus_Src_Color = GL_ONE_MINUS_SRC_COLOR,
	dstColor = GL_DST_COLOR,
	One_Minus_Dst_Color = GL_ONE_MINUS_DST_COLOR,
	SrcAlpha = GL_SRC_ALPHA,
	One_Minus_Src_Alpha = GL_ONE_MINUS_SRC_ALPHA,
	DstAlpha = GL_DST_ALPHA,
	One_Minus_Dst_Alpha = GL_ONE_MINUS_DST_ALPHA,
	ConstColor = GL_CONSTANT_COLOR,
	One_Minus_Const_Color = GL_ONE_MINUS_CONSTANT_COLOR,
	ConstAlpha =  GL_CONSTANT_ALPHA,
	One_Minus_Const_Alpha = GL_ONE_MINUS_CONSTANT_ALPHA,
	SrcAlphaSaturate = GL_SRC_ALPHA_SATURATE,
	Src1Color = GL_SRC1_COLOR,
	One_Minus_Src1_Color = GL_ONE_MINUS_SRC1_COLOR,
	Src1Alpha = GL_SRC1_ALPHA,
	One_Minus_Src1_Alpha = GL_ONE_MINUS_SRC1_ALPHA,
};

enum class BlendFunc {
	Add = GL_FUNC_ADD,
	Subtract = GL_FUNC_SUBTRACT,
	ReverseSubtract = GL_FUNC_REVERSE_SUBTRACT,
	Min = GL_MIN,
	Max = GL_MAX,
};


struct GPUPipelineState {
	GPUPipelineState();
	GPUPipelineState(const GPUPipelineState& other) = default;
	GPUPipelineState& operator = (const GPUPipelineState& other) = default;
	
	static GPUPipelineState s_defaultState;

	// clipping state
	CullFaceMode cullMode;
	FaceWindingOrder cullFaceWindingOrder;

	// rasterization state
	ShadeMode shadeMode;
	FillMode fillMode;

	// depth/stancil state
	DepthMode depthMode;
	DepthFunc depthFunc;
	int depthMask;
	StencilMode stencilMode;
	StencilOp stencilPassOp;
	StencilOp stencilFailOp;
	StencilOp stencilDepthFailOp;
	int stencilMask;
	

	// blend state
	BlendMode blendMode;
	BlendFactor blendSrcFactor;
	BlendFactor blendDstFactor;
	BlendFunc blendFunc;
	float blendColor[4];
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

