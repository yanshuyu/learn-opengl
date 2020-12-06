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
, _frameAlloc()
, m_pixels(&_frameAlloc) {

}


HDRFilter::~HDRFilter() {
	m_pixels.clear();
	_frameAlloc.clearAllFrame();
}


bool HDRFilter::initialize() {
	auto renderSize = m_manager->getRenderer()->getRenderSize();
	_frameAlloc.markFrame();
	m_pixels.reserve(3000 * 2000);
	m_pixels.resize(size_t(renderSize.x * renderSize.y));
	return true;
}


void HDRFilter::apply(Texture* inputFrame, Texture* outputFrame, const FilterParamGroup* params) {
	if (!inputFrame || !outputFrame || !params)
		return;

	m_outputTarget.detachAllTexture();
	m_outputTarget.attachProxyTexture(outputFrame, RenderTarget::Slot::Color);
	m_manager->getRenderer()->pushRenderTarget(&m_outputTarget);
	m_manager->getRenderer()->clearScreen(ClearFlags::Color);

	auto shaderWeak = ShaderProgramManager::getInstance()->getProgram("HDR");
	if (shaderWeak.expired())
		shaderWeak = ShaderProgramManager::getInstance()->addProgram("HDR");
	
	ASSERT(!shaderWeak.expired());
	auto shader = shaderWeak.lock();
	m_manager->getRenderer()->pushShaderProgram(shader.get());

	inputFrame->bind();

	shader->setUniform1("u_hdrTexture", int(Texture::Unit::Defualt));
	shader->setUniform1("u_exposure", params->getParam<float>(sExposureParamName)->m_value);
	shader->setUniform1("u_white", params->getParam<float>(sWhiteParamName)->m_value);
	shader->setUniform1("u_lumAve", calcLogAverageLuminance(inputFrame));

	m_manager->getRenderer()->drawFullScreenQuad();

	inputFrame->unbind();
	m_manager->getRenderer()->popShadrProgram();
	m_manager->getRenderer()->popRenderTarget();
}


void HDRFilter::onRenderSizeChange(float w, float h) {
	m_pixels.clear();
	_frameAlloc.clearFrame();
	
	_frameAlloc.markFrame();
	m_pixels.resize(size_t(w * h));
}


float HDRFilter::calcLogAverageLuminance(Texture* frame) {
	frame->getPiexls(0, Texture::Format::RGB, Texture::FormatDataType::Float, m_pixels.size() * sizeof(Piexl), m_pixels.data());

	float lumSum = 0;
	for (auto& piexl : m_pixels) {
		lumSum += logf(glm::dot(piexl, glm::vec3(0.2126f, 0.7152f, 0.0722f)) + 0.00001f);
	}

	return expf(lumSum / m_pixels.size());
}