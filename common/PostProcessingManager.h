#pragma once
#include"Texture.h"
#include"IFilter.h"
#include<glm/glm.hpp>
#include<functional>

class Renderer;


// post processing manager manage filter chain textures 
// and render filter chain
class PostProcessingManager {
	typedef std::unordered_map<Texture::Format, std::vector<std::unique_ptr<Texture>>> TextureCacheContainer;
	typedef std::function<IFilter* (void)> FilterCreator;

public:
	PostProcessingManager(Renderer* renderer);
	~PostProcessingManager();

	//
	// processing chain
	//
	bool initialize();
	void cleanUp();
	Texture* applyFilters(Texture* inputFrame, const FilterComponent** filters, size_t numFilters);

	//
	// texture cache 
	//
	std::unique_ptr<Texture> dequeReuseableTexture(Texture::Format fmt);
	void enqueReuseableTexture(std::unique_ptr<Texture>&& tex);
	
	inline void enqueReuseableTexture(Texture* tex) {
		enqueReuseableTexture(std::unique_ptr<Texture>(tex));
	}

	//
	// filter factory
	//
	void registerStandardFilters();

	inline void addFilterCreater(const std::string& name, FilterCreator creator) {
		m_filterFactory[name] = creator;
	}
	
	inline void removeFilterCreater(const std::string& name) {
		auto pos = m_filterFactory.find(name);
		if (pos != m_filterFactory.end())
			m_filterFactory.erase(pos);
	}

	//
	// events
	//
	void onRenderSizeChange(float w, float h);

	//
	// getter & setter
	//
	inline Renderer* getRenderer() const {
		return m_renderer;
	}

protected:
	IFilter* getFilterProcessor(const std::string& name);

protected:
	Renderer* m_renderer;
	std::array<std::unique_ptr<Texture>, 2> m_swapBuffers;
	
	TextureCacheContainer m_textureCache;

	std::unordered_map<std::string, FilterCreator> m_filterFactory;
	std::unordered_map<std::string, std::unique_ptr<IFilter>> m_filterProcessers;
};