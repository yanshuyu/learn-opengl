#include"LightComponent.h"
#include"Util.h"
#include"SceneObject.h"
#include<glm/gtx/transform.hpp>


const std::string LightComponent::s_identifier = "LightComponent";
const float LightComponent::s_maxShadowBias = 0.01f;


LightComponent::LightComponent(LightType type): Component()
, m_type(type)
, m_color(1.f, 1.f, 1.f)
, m_range(0.f)
, m_intensity(1.f)
, m_innerAngle(0.f)
, m_outterAngle(0.f)
, m_shadowType(ShadowType::NoShadow)
, m_shadowBias(-0.1f)
, m_shadowStrength(0.8f)
, m_shadowNear(1.f) {

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


bool LightComponent::isCastShadow() const {
	return m_shadowType != ShadowType::NoShadow;
}


Light_t LightComponent::makeLight() const {
	Light_t light;
	light.type = m_type;
	light.position = getPosition();
	light.direction = getDirection();
	light.color = m_color;
	light.range = m_range;
	light.innerCone = glm::radians(m_innerAngle);
	light.outterCone = glm::radians(m_outterAngle);
	light.intensity = m_intensity;

	light.shadowCamera = makeCamere();
	light.shadowType = m_shadowType;
	light.shadowBias = m_shadowBias * s_maxShadowBias;
	light.shadowStrength = m_shadowStrength;

	return light;
}


Camera_t LightComponent::makeCamere() const {
	if (m_shadowType == ShadowType::NoShadow)
		return Camera_t();

	if (m_type == LightType::SpotLight) {
#ifdef _DEBUG
		ASSERT(m_owner);
#endif // _DEBUG
		glm::vec3 pos(0.f, 10.f, 0.f);
		glm::vec3 look(0.f, 0.f, 0.f);
		glm::vec3 up(0.f, 1.f, 0.f);
		m_owner->m_transform.getCartesianAxesWorld(&pos, nullptr, &up, &look);

		Camera_t lightCam;
		lightCam.near = m_shadowNear;
		lightCam.far = m_range;
		lightCam.position = pos;
		lightCam.viewMatrix = glm::lookAt(pos, pos + look, up);
		lightCam.projMatrix = glm::perspective(glm::radians(m_outterAngle * 1.8f), 1.f, m_shadowNear, m_range);
	
		return lightCam;
	}

	return Camera_t();
}