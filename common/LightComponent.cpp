#include"LightComponent.h"
#include"Util.h"
#include"SceneObject.h"
#include<glm/gtx/transform.hpp>


const std::string LightComponent::s_identifier = "LightComponent";
const float LightComponent::s_maxShadowBias = 0.01f;


LightComponent::LightComponent(LightType type): Component()
, m_type(type)
, m_color(1.f, 1.f, 1.f)
, m_range(50.f)
, m_intensity(1.f)
, m_innerAngle(0.f)
, m_outterAngle(0.f)
, m_shadowType(ShadowType::NoShadow)
, m_shadowBias(0.f)
, m_shadowStrength(0.8f)
, m_shadowNear(1.f) {
	if (m_type == LightType::DirectioanalLight)
		m_range = 1000.f;
	if (m_type == LightType::SpotLight) {
		m_innerAngle = 45.f;
		m_outterAngle = 65.f;
	}
}


LightComponent* LightComponent::create() {
	return new LightComponent();
}


void LightComponent::destory(const LightComponent* l) {
	if (l) {
		delete l;
		l = nullptr;
	}
}


std::string LightComponent::identifier() const {
	return s_identifier;
}


Component* LightComponent::copy() const {
	LightComponent* copy = new LightComponent();
	copy->m_type = m_type;
	copy->m_color = m_color;
	copy->m_intensity = m_intensity;
	copy->m_range = m_range;
	copy->m_innerAngle = m_innerAngle;
	copy->m_outterAngle = m_outterAngle;
	copy->m_isEnable = m_isEnable;

	return copy;
}


glm::vec3 LightComponent::getPosition() const {
	if (m_type == LightType::DirectioanalLight)
		return glm::vec3(0.f);

#ifdef _DEBUG
	ASSERT(m_owner);
#endif // _DEBUG
	glm::vec3 pos(0);
	m_owner->m_transform.getCartesianAxesWorld(&pos, nullptr, nullptr, nullptr);

	return pos;
}


glm::vec3 LightComponent::getDirection() const {
	if (m_type == LightType::PointLight)
		return glm::vec3(0.f);

#ifdef _DEBUG
	ASSERT(m_owner);
#endif // _DEBUG
	glm::vec3 direction(1.f, -1.f, -1.f);
	m_owner->m_transform.getCartesianAxesWorld(nullptr, nullptr, nullptr, &direction);
	
	return direction;
}


bool LightComponent::isCastShadow() const {
	return m_shadowType != ShadowType::NoShadow;
}
