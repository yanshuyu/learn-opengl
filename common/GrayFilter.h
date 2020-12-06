#pragma once
#include"IFilter.h"
#include"RenderTarget.h"


class GrayFilterComponent : public FilterComponent {
public:
	RTTI_DECLARATION(GrayFilterComponent)

	GrayFilterComponent();

	Component* copy() const override { return nullptr;  }

	void render(RenderContext* context) override;
};




class GrayFilter : public IFilter {
public:
	GrayFilter(PostProcessingManager* mgr);

	static const std::string sName;

	void apply(Texture* inputFrame, Texture* outputFrame, const FilterParamGroup* params) override;

protected:
	RenderTarget m_outputTarget;
};