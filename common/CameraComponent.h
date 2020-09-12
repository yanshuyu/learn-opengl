#pragma once
#include"Component.h"
#include"RendererCore.h"
#include<glm/glm.hpp>


class Scene;
class TextureCubeApp;

class CameraComponent : public Component {
	friend class Scene;
	friend class TextureCubeApp; // for testing pupose
public:
	enum class ProjectionMode {
		Perspective,
		Orthogonal,
	};

	struct Viewport {
		float x;
		float y;
		float width;
		float height;

		Viewport();
		Viewport(float _x, float _y, float _w, float _h);
	};

protected:
	CameraComponent(float wndWidth, float wndHeight);

public:
	~CameraComponent();

	CameraComponent(const CameraComponent& other) = delete;
	CameraComponent(CameraComponent&& rv) = delete;
	CameraComponent& operator = (const CameraComponent& other) = delete;
	CameraComponent& operator = (CameraComponent&& rv) = delete;

	static const std::string s_indentifier;

	std::string indentifier() const override;
	Component* copy() const override;
	void setWindowSize(float w, float h);

	glm::mat4 viewMatrix() const;
	glm::mat4 projectionMatrix() const;
	glm::mat4 viewProjectionMatrix() const;

	Camera_t makeCamera() const;

public:	
	float m_fov;
	float m_near;
	float m_far;
	glm::vec4 m_backGroundColor;

	Viewport m_viewport;
	ProjectionMode m_projMode;

private: 
	float m_windowWidth;
	float m_windowHeight;
};