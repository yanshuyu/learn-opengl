#include"LightComponent.h"
#include"Util.h"
#include"SceneObject.h"
#include"Scene.h"
#include<glm/gtx/transform.hpp>


RTTI_IMPLEMENTATION(LightComponent)

const float LightComponent::s_maxShadowBias = 0.01f;


LightComponent::LightComponent(LightType type): Component()
, m_type(type)
, m_color(1.f, 1.f, 1.f)
, m_range(50.f)
, m_intensity(1.f)
, m_innerAngle(0.f)
, m_outterAngle(0.f)
, m_shadowType(ShadowType::NoShadow)
, m_shadowBias(0.1f)
, m_shadowStrength(0.8f) {
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


void LightComponent::onAttached() {
	__super::onAttached();
	m_owner->getParentScene()->onLightAdded(m_owner, this);
}


void LightComponent::onDetached() {
	__super::onDetached();
	m_owner->getParentScene()->onLightRemoved(m_owner, this);
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


Light_t LightComponent::makeLight() const {
	Light_t l;
	l.type = m_type;
	l.position = getPosition();
	l.direction =	getDirection();
	l.color = m_color;
	l.range = m_range;
	l.innerCone = glm::radians(m_innerAngle);
	l.outterCone = glm::radians(m_outterAngle);
	l.intensity = m_intensity;

	l.shadowType = m_shadowType;
	l.shadowBias = m_shadowBias * LightComponent::s_maxShadowBias;
	l.shadowStrength = m_shadowStrength;

	return l;
}