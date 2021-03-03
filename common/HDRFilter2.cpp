#include"HDRFilter2.h"
#include"Renderer.h"
#include"PostProcessingManager.h"
#include"ShaderProgamMgr.h"


static const char* sHDRFilter2Name = "HDR2";
static const char* sExposureParamName = "Exposure";

RTTI_IMPLEMENTATION(HDRFilterComponent2)

HDRFilterComponent2::HDRFilterComponent2() :FilterComponent(sHDRFilter2Name) 
, m_exposure(1.0f) {

}

bool HDRFilterComponent2::initialize() {
	m_params.addParam<float>(sExposureParamName, m_exposure);
	return true;
}



void HDRFilterComponent2::render(RenderContext* context) {
	m_params.getParam<float>(sExposureParamName)->m_value = m_exposure;
	context->getRenderer()->submitPostProcessingFilter(this);
}




const std::string HDRFilter2::sName = sHDRFilter2Name;


HDRFilter2::HDRFilter2(PostProcessingManager* mgr) : IFilter(sHDRFilter2Name, mgr)
, m_outputTarget(mgr->getRenderer()->getRenderSize()) {

}


void HDRFilter2::apply(Texture* inputFrame, Texture* outputFrame, const FilterParamGroup* params) {
	if (!inputFrame || !outputFrame || !params)
		return;

	auto shader_weak = ShaderProgramManager::getInstance()->getProgram("HDR2");
	if (shader_weak.expired())
		shader_weak = ShaderProgramManager::getInstance()->addProgram("HDR2");

	ASSERT(!shader_weak.expired());
	
	m_outputTarget.detachAllTexture();
	m_outputTarget.attachProxyTexture(outputFrame, RenderTarget::Slot::Color);
	m_manager->getRenderer()->pushRenderTarget(&m_outputTarget);
	m_manager->getRenderer()->clearScreen(ClearFlags::Color);

	auto shader = shader_weak.lock();
	m_manager->getRenderer()->pushShaderProgram(shader.get());
	
	inputFrame->bindToTextureUnit(Texture::Unit::Defualt, Texture::Target::Texture_2D);
	shader->setUniform1("u_hdrTexture", int(Texture::Unit::Defualt));
	shader->setUniform1("u_Exposure", params->getParam<float>(sExposureParamName)->m_value);

	m_manager->getRenderer()->drawFullScreenQuad();

	inputFrame->unbindFromTextureUnit();
	m_manager->getRenderer()->popShadrProgram();
	m_manager->getRenderer()->popRenderTarget();
}