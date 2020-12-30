#include"LightComponent.h"
#include"Util.h"
#include"SceneObject.h"
#include"Scene.h"
#include"Renderer.h"
#include<glm/gtx/transform.hpp>


RTTI_IMPLEMENTATION(LightComponent)

const float LightComponent::s_maxShadowBias = 0.1f;


LightComponent::LightComponent(LightType type): RenderableComponent()
, m_type(type)
, m_color(1.f, 1.f, 1.f)
, m_range(50.f)
, m_intensity(1.f)
, m_innerAngle(0.f)
, m_outterAngle(0.f)
, m_shadowType(ShadowType::NoShadow)
, m_shadowBias(0.f)
, m_shadowStrength(0.8f) {
	if (m_type == LightType::DirectioanalLight)
		m_range = 1000.f;
	if (m_type == LightType::SpotLight) {
		m_innerAngle = 45.f;
		m_outterAngle = 65.f;
	}
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


void LightComponent::render(RenderContext* context) {
	context->getRenderer()->submitLight(makeLight());
}


Light_t LightComponent::makeLight() const {
	Light_t l;

	l.type = m_type;
	l.position = getPosition();
	l.direction = getDirection();
	l.color = m_color;
	l.range = m_range;
	l.intensity = m_intensity;
	l.innerCone = glm::radians(m_innerAngle);
	l.outterCone = glm::radians(m_outterAngle);
	l.shadowType = m_shadowType;
	l.shadowBias = m_shadowBias * LightComponent::s_maxShadowBias;
	l.shadowStrength = m_shadowStrength;

	if (m_type == LightType::DirectioanalLight) {
		l.range = std::numeric_limits<float>::max();
	}

	return l;
}