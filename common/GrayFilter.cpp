#include"GrayFilter.h"
#include"Renderer.h"
#include"ShaderProgamMgr.h"

const static char* sGrayFilterName = "Gray";

RTTI_IMPLEMENTATION(GrayFilterComponent)

GrayFilterComponent::GrayFilterComponent() :FilterComponent(sGrayFilterName) {

}


void GrayFilterComponent::render(RenderContext* context) {
	context->getRenderer()->submitPostProcessingFilter(this);
}



const std::string GrayFilter::sName = sGrayFilterName;

GrayFilter::GrayFilter(PostProcessingManager* mgr) : IFilter(sGrayFilterName, mgr)
, m_outputTarget(mgr->getRenderer()->getRenderSize()) {

}


void GrayFilter::apply(Texture* inputFrame, Texture* outputFrame, const FilterParamGroup* params) {	
	if (!inputFrame || !outputFrame)
		return;

	auto shader_weak = ShaderProgramManager::getInstance()->getProgram("Gray");
	if (shader_weak.expired())
		shader_weak = ShaderProgramManager::getInstance()->addProgram("Gray");

	ASSERT(!shader_weak.expired());
	
	auto shader = shader_weak.lock();
	m_manager->getRenderer()->pushShaderProgram(shader.get());

	m_outputTarget.detachAllTexture();
	m_outputTarget.attachProxyTexture(outputFrame, RenderTarget::Slot::Color);
	m_manager->getRenderer()->pushRenderTarget(&m_outputTarget);

	inputFrame->bindToTextureUnit(Texture::Unit::Defualt, Texture::Target::Texture_2D);
	shader->setUniform1("u_Texture", int(Texture::Unit::Defualt));

	m_manager->getRenderer()->drawFullScreenQuad();
	
	inputFrame->unbindFromTextureUnit();
	m_manager->getRenderer()->popRenderTarget();
	m_manager->getRenderer()->popShadrProgram();
}