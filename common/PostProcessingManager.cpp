#include"PostProcessingManager.h"
#include"Renderer.h"
#include<algorithm>
#include"HDRFilter.h"
#include"HDRFilter2.h"
#include"GrayFilter.h"
#include"GaussianBlurFilter.h"


PostProcessingManager::PostProcessingManager(Renderer* renderer) : m_renderer(renderer)
, m_swapBuffers()
, m_textureCache()
, m_filterFactory()
, m_filterProcessers() {
}


PostProcessingManager::~PostProcessingManager() {
	cleanUp();
	m_filterFactory.clear();
	m_filterProcessers.clear();
}


bool PostProcessingManager::initialize() {
	auto frontBuffer = std::unique_ptr<Texture>(new Texture());
	auto backBuffer = std::unique_ptr<Texture>(new Texture());
	auto renderSz = m_renderer->getRenderSize();

	frontBuffer->bindToTextureUnit();
	if (!frontBuffer->loadImage2DFromMemory(Texture::Format::RGBA, Texture::Format::RGBA, Texture::FormatDataType::UByte, renderSz.x, renderSz.y, nullptr)) {
		cleanUp();
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return false;
	}
	frontBuffer->unbindFromTextureUnit();

	backBuffer->bindToTextureUnit();
	if (!backBuffer->loadImage2DFromMemory(Texture::Format::RGBA, Texture::Format::RGBA, Texture::FormatDataType::UByte, renderSz.x, renderSz.y, nullptr)) {
		cleanUp();
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return false;
	}
	backBuffer->unbindFromTextureUnit();

	m_swapBuffers[0] = std::move(frontBuffer);
	m_swapBuffers[1] = std::move(backBuffer);

	return true;
}


void PostProcessingManager::cleanUp() {
	m_swapBuffers[0] = nullptr;
	m_swapBuffers[1] = nullptr;
	m_textureCache.clear();
}


Texture* PostProcessingManager::applyFilters(Texture* inputFrame, const FilterComponent** filters, size_t numFilters) {
	if (numFilters <= 0)
		return inputFrame;

	m_renderer->pushGPUPipelineState(&GPUPipelineState::s_defaultState);

	Texture* inFrame = nullptr;
	Texture* outFrame = nullptr;
	for (size_t i = 0; i < numFilters; i++) {
		inFrame = outFrame ? outFrame : inputFrame;
		outFrame = m_swapBuffers[i % 2].get();
		IFilter* processor = getFilterProcessor(filters[i]->getName());
		ASSERT(processor);
		processor->apply(inFrame, outFrame, filters[i]->getParams());
	}

	m_renderer->popGPUPipelineState();

	return outFrame;
}


std::unique_ptr<Texture> PostProcessingManager::dequeReuseableTexture(Texture::Format fmt) {
	if (m_textureCache.find(fmt) == m_textureCache.end())
		return nullptr;

	auto& texList = m_textureCache.at(fmt);
	if (texList.empty())
		return nullptr;

	auto last = std::move(texList.back());
	texList.pop_back();

	return last;
}


void PostProcessingManager::enqueReuseableTexture(std::unique_ptr<Texture>&& tex) {
	auto fmt = tex->getFormat();
	if (m_textureCache.find(fmt) == m_textureCache.end())
		m_textureCache[fmt] = std::vector<std::unique_ptr<Texture>>();
	
	m_textureCache.at(fmt).push_back(std::move(tex));
}


void PostProcessingManager::onRenderSizeChange(float w, float h) {
	cleanUp();
	bool ok = initialize();

#ifdef _DEBUG
	ASSERT(ok);
#endif // _DEBUG

	for (auto& filter : m_filterProcessers) {
		filter.second->onRenderSizeChange(w, h);
	}
}


IFilter* PostProcessingManager::getFilterProcessor(const std::string& name) {
	if (m_filterProcessers.find(name) != m_filterProcessers.end())
		return m_filterProcessers.at(name).get();

	if (m_filterFactory.find(name) == m_filterFactory.end())
		return nullptr;

	auto filter = std::unique_ptr<IFilter>(m_filterFactory.at(name)());
	if (!filter->initialize())
		return nullptr;

	m_filterProcessers[name] = std::move(filter);

	return m_filterProcessers.at(name).get();
}


void PostProcessingManager::registerStandardFilters() {
	addFilterCreater(HDRFilter::sName, [this]()->IFilter* { return new HDRFilter(this); });
	addFilterCreater(HDRFilter2::sName, [this]()->IFilter* { return new HDRFilter2(this); });
	addFilterCreater(GrayFilter::sName, [this]()->IFilter* { return new GrayFilter(this); });
	addFilterCreater(GaussianBlurFilter::sName, [this]()->IFilter* { return new GaussianBlurFilter(this); });
}
	