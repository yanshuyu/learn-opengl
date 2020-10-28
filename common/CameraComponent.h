#pragma once
#include"Component.h"
#include"RendererCore.h"
#include"Util.h"
#include<glm/glm.hpp>



class Scene;
class TextureCubeApp;

class CameraComponent : public Component {
	friend class Scene;
	
	RTTI_DECLARATION(CameraComponent)

public:
	enum class ProjectionMode {
		Perspective,
		Orthogonal,
	};

public:
	CameraComponent(float ar = 1.f);
	~CameraComponent();

	CameraComponent(const CameraComponent& other) = delete;
	CameraComponent(CameraComponent&& rv) = delete;
	CameraComponent& operator = (const CameraComponent& other) = delete;
	CameraComponent& operator = (CameraComponent&& rv) = delete;

	Component* copy() const override;

	glm::mat4 viewMatrix() const;
	glm::mat4 projectionMatrix() const;
	glm::mat4 viewProjectionMatrix() const;

	glm::vec3 getPosition() const;
	glm::vec3 getLookDirection() const;
	Viewport_t getViewPort(float renderWidth, float renderHeight) const; // screen space view port
	ViewFrustum_t getViewFrustum() const; // view space view frumstum
	
protected:
	Camera_t makeCamera(float wndWidth, float wndHeight) const;

public:	
	// persp
	float m_fov;
	float m_aspectRatio;
	float m_near;
	float m_far;

	// ortho
	float m_left;
	float m_right;
	float m_top;
	float m_bottom;

	// normalized viewport
	float m_viewPortX;
	float m_viewPortY;
	float m_viewPortW;
	float m_viewPortH;
	
	glm::vec4 m_backGroundColor;
	ProjectionMode m_projMode;
};