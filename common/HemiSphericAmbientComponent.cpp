#include"HemiSphericAmbientComponent.h"
#include"Renderer.h"


RTTI_IMPLEMENTATION(HemiSphericAmbientComponent)


HemiSphericAmbientComponent::HemiSphericAmbientComponent() : RenderableComponent()
, m_skyAmbient(0.f, 0.f, 0.1f)
, m_landAmbient(0.f, 0.1f, 0.f)
, m_intensity(1.f){

}


void HemiSphericAmbientComponent::render(RenderContext* ctx) {
	ctx->getRenderer()->submitLight(makeLight());
}



Light_t HemiSphericAmbientComponent::makeLight() const {
	Light_t light;
	light.type = LightType::Ambient;
	light.position = glm::vec3(0.f);
	light.range = std::numeric_limits<float>::max();
	light.color = m_skyAmbient;
	light.colorEx = m_landAmbient;
	light.intensity = m_intensity;
	light.shadowType = ShadowType::NoShadow;
	
	return light;
}