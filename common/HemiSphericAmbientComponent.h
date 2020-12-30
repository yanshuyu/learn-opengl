#pragma once
#include"RenderableComponent.h"
#include"RendererCore.h"

class HemiSphericAmbientComponent : public RenderableComponent {
	RTTI_DECLARATION(HemiSphericAmbientComponent)

public:
	HemiSphericAmbientComponent();
	~HemiSphericAmbientComponent() {}

	inline Component* copy() const override { return	nullptr; }
	inline void render(RenderContext* context) override;

protected:
	Light_t makeLight() const;

public:
	glm::vec3 m_skyAmbient;
	glm::vec3 m_landAmbient;
	float m_intensity;
};