#pragma once
#include"Component.h"
#include"RendererCore.h"
#include<glm/glm.hpp>


class Scene;
class TextureCubeApp;

class CameraComponent : public Component {
	friend class Scene;
	
public:
	enum class ProjectionMode {
		Perspective,
		Orthogonal,
	};

protected:
	CameraComponent(float wndWidth, float wndHeight);

public:
	~CameraComponent();

	CameraComponent(const CameraComponent& other) = delete;
	CameraComponent(CameraComponent&& rv) = delete;
	CameraComponent& operator = (const CameraComponent& other) = delete;
	CameraComponent& operator = (CameraComponent&& rv) = delete;

	static const std::string s_identifier;

	std::string identifier() const override;
	Component* copy() const override;
	void onWindowSizeChange(float w, float h);

	glm::mat4 viewMatrix() const;
	glm::mat4 projectionMatrix() const;
	glm::mat4 viewProjectionMatrix() const;

	glm::vec3 getPosition() const;
	glm::vec3 getLookDirection() const;
	Viewport_t getViewPort() const; // screen space view port
	ViewFrustum_t getViewFrustum() const; // world space view frumstum

public:	
	// persp
	float m_fov;
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

private: 
	float m_windowWidth;
	float m_windowHeight;
};