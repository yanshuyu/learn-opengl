#pragma once
#include"FilterParams.h"
#include"RenderableComponent.h"
#include<string>


// base filter component class for attach to game object
class FilterComponent: public RenderableComponent {
	RTTI_DECLARATION(FilterComponent)

public:
	FilterComponent(const std::string& name);
	virtual ~FilterComponent() {};

	inline std::string getName() const {
		return m_name;
	}

	inline const FilterParamGroup* getParams() const {
		return &m_params;
	}

protected:
	const std::string m_name;
	FilterParamGroup m_params;
};



class Texture;
class Renderer;
class PostProcessingManager;

// base filter class for rendering
class IFilter {
public:
	IFilter(const std::string& name, PostProcessingManager* mgr);
	virtual ~IFilter() {}

	virtual bool initialize() { return true; }

	virtual void cleanUp() {}

	virtual void apply(Texture* inputFrame, Texture* outputFrame, const FilterParamGroup* params) = 0;

	virtual void onRenderSizeChange(float w, float h) {};

protected:
	PostProcessingManager* m_manager;
	const std::string m_name;
};


