#pragma once
#include"RenderableComponent.h"
#include"RendererCore.h"
#include"Util.h"
#include<glm/glm.hpp>



class Scene;
class TextureCubeApp;

class CameraComponent : public RenderableComponent {
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

	inline Component* copy() const override { return nullptr; }
	inline void render(RenderContext* context) override;

	virtual void onAttached() override;
	virtual void onDetached() override;

	glm::mat4 viewMatrix() const;
	glm::mat4 projectionMatrix() const;
	glm::mat4 viewProjectionMatrix() const;

	glm::vec3 getPosition() const;
	glm::vec3 getLookDirection() const;
	Viewport_t getViewPort(const glm::vec2& renderSize) const; // screen space view port
	ViewFrustum_t getViewFrustum() const; // view space view frumstum
	
	inline void onWindowResized(const glm::vec2& sz) {
		if (sz.y != 0) m_aspectRatio = sz.x / sz.y;
	}

protected:
	Camera_t makeCamera(const glm::vec2& renderSize) const;

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
	mutable float m_viewPortMinX;
	mutable float m_viewPortMinY;
	mutable float m_viewPortMaxX;
	mutable float m_viewPortMaxY;
	
	glm::vec4 m_clearColor;
	ProjectionMode m_projMode;
};