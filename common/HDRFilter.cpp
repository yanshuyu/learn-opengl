#include"HDRFilter.h"
#include"RendererCore.h"
#include"Renderer.h"
#include"ShaderProgamMgr.h"


static const char* sHDRFilterName = "HDR";
static const char* sExposureParamName = "Exposure";
static const char* sWhiteParamName = "White";

RTTI_IMPLEMENTATION(HDRFilterComponent)


HDRFilterComponent::HDRFilterComponent() : FilterComponent(sHDRFilterName)
, m_exposure(0.35f)
, m_white(0.92f){

}


bool HDRFilterComponent::initialize() {
	m_params.addParam<float>(sExposureParamName, m_exposure);
	m_params.addParam<float>(sWhiteParamName, m_white);

	return true;
}


Component* HDRFilterComponent::copy() const {
	return nullptr;
}


void HDRFilterComponent::render(RenderContext* context) {
	m_params.getParam<float>(sExposureParamName)->m_value = m_exposure;
	m_params.getParam<float>(sWhiteParamName)->m_value = m_white;
	context->getRenderer()->submitPostProcessingFilter(this);
}




const std::string HDRFilter::sName = sHDRFilterName;

HDRFilter::HDRFilter(PostProcessingManager* mgr) :IFilter(sHDRFilterName, mgr)
, m_outputTarget(mgr->getRenderer()->getRenderSize())
, m_histBuffer()
, m_aveLumTex()
, m_atomicCounter() {

}


HDRFilter::~HDRFilter() {
	m_histBuffer.release();
	m_aveLumTex.release();
}


bool HDRFilter::initialize() {
	m_histBuffer.bind(Buffer::Target::ShaderStorageBuffer);
	if (!m_histBuffer.loadData(nullptr, sizeof(unsigned int) * 256, Buffer::Usage::DynamicDraw)) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return false;
	}
	m_histBuffer.unbind();

	m_atomicCounter.bind(Buffer::Target::AtomicCounterBuffer);
	m_atomicCounter.loadData(nullptr, sizeof(unsigned int), Buffer::Usage::DynamicDraw);
	m_atomicCounter.unbind();

	m_aveLumTex.bindToTextureUnit(Texture::Unit::Defualt, Texture::Target::Texture_2D);
	m_aveLumTex.allocStorage2D(Texture::Format::R16F, 1, 1);
	m_aveLumTex.unbindFromTextureUnit();

	return true;
}


void HDRFilter::apply(Texture* inputFrame, Texture* outputFrame, const FilterParamGroup* params) {
	if (!inputFrame || !outputFrame || !params)
		return;

	// calculate  histogram
	auto shader = ShaderProgramManager::getInstance()->getProgram("HDRHistogram");
	if (shader.expired()) {
		shader = ShaderProgramManager::getInstance()->addProgram("HDRHistogram");
		ASSERT(!shader.expired())
	}
	auto histShader = shader.lock();
	m_manager->getRenderer()->pushShaderProgram(histShader.get());

	m_histBuffer.bindBase(Buffer::Target::ShaderStorageBuffer, 0);
	m_atomicCounter.bindBase(Buffer::Target::AtomicCounterBuffer, 0);

	auto bufferData = (unsigned int*)m_histBuffer.map(Buffer::MapAccess::Write);
	memset(bufferData, 0, sizeof(unsigned int) * 256);
	m_histBuffer.unmap();

	bufferData = (unsigned int*)m_atomicCounter.map(Buffer::MapAccess::Write);
	*bufferData = 0;
	m_atomicCounter.unmap();

	inputFrame->bindToImageUnit(0, Texture::Format::RGBA16F, Texture::Access::Read);

	histShader->setUniform1("u_InputImage", 0);
	histShader->bindShaderStorageBlock("Hist", 0);

	auto renderSz = m_manager->getRenderer()->getRenderSize();
	m_manager->getRenderer()->dispatchCompute(glm::ceil(renderSz.x / 16), glm::ceil(renderSz.y / 16));

	unsigned numPix = (*(unsigned int*)m_atomicCounter.map(Buffer::MapAccess::Read));
	m_atomicCounter.unmap();
	m_atomicCounter.unbind();
	inputFrame->unbindFromImageUnit();
	m_manager->getRenderer()->popShadrProgram();

	// calculate average log luminance
	shader = ShaderProgramManager::getInstance()->getProgram("HDRAveLogLuminance");
	if (shader.expired()) {
		shader = ShaderProgramManager::getInstance()->addProgram("HDRAveLogLuminance");
		ASSERT(!shader.expired());
	}
	auto aveLumShader = shader.lock();
	m_manager->getRenderer()->pushShaderProgram(aveLumShader.get());
	
	m_aveLumTex.bindToImageUnit(0, Texture::Format::R16F, Texture::Access::ReadWrite);

	aveLumShader->setUniform1("u_OutAveLum", 0);
	aveLumShader->bindShaderStorageBlock("Hist", 0);
	aveLumShader->setUniform1("u_NumPixel", numPix);

	m_manager->getRenderer()->dispatchCompute(256);

	m_histBuffer.unbind();
	m_aveLumTex.unbindFromImageUnit();
	m_manager->getRenderer()->popShadrProgram();
	
	m_outputTarget.detachAllTexture();
	m_outputTarget.attachProxyTexture(outputFrame, RenderTarget::Slot::Color);
	m_manager->getRenderer()->pushRenderTarget(&m_outputTarget);
	m_manager->getRenderer()->clearScreen(ClearFlags::Color);

	shader = ShaderProgramManager::getInstance()->getProgram("HDR");
	if (shader.expired()) {
		shader = ShaderProgramManager::getInstance()->addProgram("HDR");
		ASSERT(!shader.expired());
	}

	auto hdrShader = shader.lock();
	m_manager->getRenderer()->pushShaderProgram(hdrShader.get());

	m_aveLumTex.bindToTextureUnit(Texture::Unit::Defualt);
	inputFrame->bindToTextureUnit(Texture::Unit::DiffuseMap);
	hdrShader->setUniform1("u_aveLumTexture", int(Texture::Unit::Defualt));
	hdrShader->setUniform1("u_hdrTexture", int(Texture::Unit::DiffuseMap));
	hdrShader->setUniform1("u_exposure", params->getParam<float>(sExposureParamName)->m_value);
	hdrShader->setUniform1("u_white", params->getParam<float>(sWhiteParamName)->m_value);

	m_manager->getRenderer()->drawFullScreenQuad();

	inputFrame->unbindFromTextureUnit();
	m_manager->getRenderer()->popShadrProgram();
	m_manager->getRenderer()->popRenderTarget();
}
