#pragma once
#include"Component.h"
#include"RendererCore.h"
#include<glm/glm.hpp>



class LightComponent : public Component {
public:
	LightComponent(LightType type = LightType::DirectioanalLight);
	LightComponent(const LightComponent& other) = delete;
	LightComponent(LightComponent&& rv) = delete;

	LightComponent& operator = (const LightComponent& other) = delete;
	LightComponent& operator = (LightComponent&& rv) = delete;

	static const std::string s_identifier;
	static LightComponent* create();
	static void destory(const LightComponent* l);

	std::string identifier() const override;
	Component* copy() const override;

	glm::vec3 getPosition() const;
	glm::vec3 getDirection() const;
	Light_t makeLight() const;


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

private:
	LightType m_type;
	glm::vec3 m_color;
	float m_intensity;
	float m_range;
	float m_innerAngle;
	float m_outterAngle;
};