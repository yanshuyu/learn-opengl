#pragma once
#include"RenderableComponent.h"
#include"RendererCore.h"
#include"Util.h"
#include<glm/glm.hpp>

class Scene;

class LightComponent : public RenderableComponent {
	friend class  Scene;

	RTTI_DECLARATION(LightComponent)

public:
	LightComponent(LightType type = LightType::DirectioanalLight);

	static const float s_maxShadowBias;


	inline Component* copy() const override { return nullptr; };

	inline void render(RenderContext* context) override;

	glm::vec3 getPosition() const;
	glm::vec3 getDirection() const;
	
	inline bool isCastShadow() const {
		return m_shadowType != ShadowType::NoShadow;
	};

	inline glm::vec3 getColor() const {
		return m_color;
	}

	inline void setColor(const glm::vec3& c) {
		m_color = c;
	}

	inline float getIntensity() const {
		return m_intensity;
	}

	inline void setIntensity(float i) {
		m_intensity = i;
	}

	inline float getRange() const {
		return m_range;
	}

	inline void setRange(float r) {
		m_range = r;
	}

	inline float getSpotInnerAngle() const {
		return m_innerAngle;
	}

	inline void setSpotInnerAngle(float angle) {
		m_innerAngle = angle;
	}

	inline float getSpotOutterAngle() const {
		return m_outterAngle;
	}

	inline void setSpotOutterAngle(float angle) {
		m_outterAngle = angle;
	}

	inline LightType getType() const {
		return m_type;
	}

	inline void setType(LightType t) {
		m_type = t;
	}

	inline void setShadowType(ShadowType st) {
		m_shadowType = st;
	}

	inline ShadowType getShadowType() const {
		return m_shadowType;
	}

	inline void setShadowBias(float bias) {
		m_shadowBias = bias;
	}

	inline float getShadowBias() const {
		return m_shadowBias;
	}

	inline void setShadowStrength(float strength) {
		m_shadowStrength = strength;
	}

	inline float getShadowStrength() const {
		return m_shadowStrength;
	}

protected:
	Light_t makeLight() const;

protected:
	LightType m_type;
	glm::vec3 m_color;
	float m_intensity;
	float m_range;
	float m_innerAngle;
	float m_outterAngle;

	ShadowType m_shadowType;
	float m_shadowBias;
	float m_shadowStrength;
};