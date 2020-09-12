#pragma once
#include"VertexArray.h"
#include<glm/glm.hpp>
#include"Mesh.h"
#include"Material.h"
#include<stack>

class Renderer;


struct Camera_t {
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
	glm::vec4 backgrounColor;
	float viewPortX;
	float viewportY;
	float viewportWidth;
	float viewportHeight;
};



struct Light_t {
	enum class LightType {
		DirectionalLight,
		PointLight,
		SpotLight,
		CapsuleLight,
	};

	LightType lightType;
	glm::vec3 direction;
	glm::vec3 color;
	glm::vec3 position;
	float range;
	float innerCone;
	float outterCone;
	float intensity;
	float length;
};



struct RenderTask_t {
	const VertexArray* vao;
	const Material* material;
	size_t indexCount;
	size_t vertexCount;
	PrimitiveType primitive;
	glm::mat4 modelMatrix;
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




