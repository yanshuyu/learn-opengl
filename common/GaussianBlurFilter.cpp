#include"GaussianBlurFilter.h"
#include"Renderer.h"
#include"PostProcessingManager.h"
#include"ShaderProgamMgr.h"
#include<glm/gtc/constants.hpp>


static const char* sGaussianFilterName = "GuassianBlur";
static const char* sSigmaParamName = "Sigma";
static const char* sKernelSizeParamName = "Kernel";


RTTI_IMPLEMENTATION(GaussianBlurFilterComponent)

const double GaussianBlurFilterComponent::sMinSigma = 0.1;
const double GaussianBlurFilterComponent::sMaxSigma = 25;
const int GaussianBlurFilterComponent::sMinKernel = 3;
const int GaussianBlurFilterComponent::sMaxKernel = 37;

GaussianBlurFilterComponent::GaussianBlurFilterComponent(): FilterComponent(sGaussianFilterName)
, m_sigma(1)
, m_kernelSize(5) {

}


bool GaussianBlurFilterComponent::initialize() {
	m_params.addParam<float>(sSigmaParamName, m_sigma);
	m_params.addParam<int>(sKernelSizeParamName, m_kernelSize);
	
	return true;
}


void GaussianBlurFilterComponent::render(RenderContext* context) {
	m_params.getParam<float>(sSigmaParamName)->m_value = m_sigma;
	m_params.getParam<int>(sKernelSizeParamName)->m_value = m_kernelSize;
	context->getRenderer()->submitPostProcessingFilter(this);
}



const std::string GaussianBlurFilter::sName = sGaussianFilterName;

GaussianBlurFilter::GaussianBlurFilter(PostProcessingManager* mgr) : IFilter(sGaussianFilterName, mgr)
, m_sigma(0)
, m_kernel(0)
, m_weights()
, m_halfBlurTarget(mgr->getRenderer()->getRenderSize())
, m_outputTarget(mgr->getRenderer()->getRenderSize()) {
	m_weights.reserve(GaussianBlurFilterComponent::sMaxKernel / 2 + 1);
}

void GaussianBlurFilter::apply(Texture* inputFrame, Texture* outputFrame, const FilterParamGroup* params) {
	calcWeights(params->getParam<float>(sSigmaParamName)->m_value, params->getParam<int>(sKernelSizeParamName)->m_value);
	// set up shader
	auto shader_weak = ShaderProgramManager::getInstance()->getProgram("GaussianBlur");
	if (shader_weak.expired())
		shader_weak = ShaderProgramManager::getInstance()->addProgram("GaussianBlur");

	ASSERT(!shader_weak.expired());
	auto shader = shader_weak.lock();
	
	m_manager->getRenderer()->pushShaderProgram(shader.get());
	shader->setUniform1("u_NumWeights", int(m_weights.size()));
	shader->setUniform1v("u_Weights[0]", m_weights.data(), m_weights.size());
	
	// horizontal blur pass
	auto halfBlurTex = m_manager->dequeReuseableTexture(Texture::Format::RGBA);
	if (!halfBlurTex) {
		auto renderSz = m_manager->getRenderer()->getRenderSize();
		halfBlurTex = std::make_unique<Texture>();
		halfBlurTex->bind();
		halfBlurTex->loadImage2DFromMemory(Texture::Format::RGBA, Texture::Format::RGBA, Texture::FormatDataType::UByte, renderSz.x, renderSz.y, nullptr);
		halfBlurTex->unbind();
	}

	ASSERT(halfBlurTex);

	m_manager->getRenderer()->pushRenderTarget(&m_halfBlurTarget);
	m_halfBlurTarget.detachAllTexture();
	m_halfBlurTarget.attachProxyTexture(halfBlurTex.get(), RenderTarget::Slot::Color);
	m_manager->getRenderer()->clearScreen(ClearFlags::Color);

	inputFrame->bind(Texture::Unit::Defualt, Texture::Target::Texture_2D);
	shader->setUniform1("u_Texture", int(Texture::Unit::Defualt));
	shader->setUniform1("u_Pass", 0);

	m_manager->getRenderer()->drawFullScreenQuad();
	
	m_manager->getRenderer()->popRenderTarget();

	// vertical blur pass
	m_manager->getRenderer()->pushRenderTarget(&m_outputTarget);
	m_outputTarget.detachAllTexture();
	m_outputTarget.attachProxyTexture(outputFrame, RenderTarget::Slot::Color);
	m_manager->getRenderer()->clearScreen(ClearFlags::Color);
	
	halfBlurTex->bind(Texture::Unit::Defualt, Texture::Target::Texture_2D);
	shader->setUniform1("u_Pass", 1);
	
	m_manager->getRenderer()->drawFullScreenQuad();

	halfBlurTex->unbind();
	inputFrame->unbind();
	m_manager->getRenderer()->popRenderTarget();
	m_manager->getRenderer()->popShadrProgram();
	m_manager->enqueReuseableTexture(std::move(halfBlurTex));
}


void GaussianBlurFilter::calcWeights(float sigma, int kernel) {
	if (sigma == m_sigma && kernel == m_kernel)
		return;
		
	float sum = 0;
	m_weights.clear();
	m_weights.resize(kernel / 2 + 1, 0);
	m_weights[0] = gaussian(sigma, 0);
	sum += m_weights[0];
	for (size_t i = 1; i < m_weights.size(); i++) {
		m_weights[i] = gaussian(sigma, i);
		sum += m_weights[i] * 2;
	}

	for (size_t i = 0; i < m_weights.size(); i++) {  // normalize weights, sum of all weights is 1
		m_weights[i] /= sum;
	}

	m_sigma = sigma;
	m_kernel = kernel;
}


float GaussianBlurFilter::gaussian(float sigma, int x) { // 1D gaussian equaltion
	float cof = 1.0 / (glm::root_two_pi<double>() * sigma);
	float p = -0.5 * (x * x) / (sigma * sigma);
	return cof * glm::exp(p);
}