#include"LightComponent.h"
#include"Util.h"
#include"SceneObject.h"


const std::string LightComponent::s_identifier = "LightComponent";

LightComponent::LightComponent(LightType type): Component()
, m_type(type)
, m_color(1.f, 1.f, 1.f)
, m_range(0.f)
, m_intensity(1.f)
, m_innerAngle(0.f)
, m_outterAngle(0.f) {

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
#ifdef _DEBUG
	ASSERT(m_owner);
#endif // _DEBUG
	glm::vec3 pos(0);
	m_owner->m_transform.getCartesianAxesWorld(&pos, nullptr, nullptr, nullptr);

	return pos;
}


glm::vec3 LightComponent::getDirection() const {
#ifdef _DEBUG
	ASSERT(m_owner);
#endif // _DEBUG
	glm::vec3 direction(1.f, -1.f, -1.f);
	m_owner->m_transform.getCartesianAxesWorld(nullptr, nullptr, nullptr, &direction);
	
	return direction;
}


Light_t LightComponent::makeLight() const {
	Light_t light;
	light.type = m_type;
	light.position = getPosition();
	light.direction = getDirection();
	light.color = m_color;
	light.range = m_range;
	light.innerCone = m_innerAngle;
	light.outterCone = m_outterAngle;
	light.intensity = m_intensity;

	return light;
}