#pragma once
#include"IFilter.h"
#include"RenderTarget.h"
#include"FrameAllocator.h"

class HDRFilterComponent: public FilterComponent {
	RTTI_DECLARATION(HDRFilterComponent)

public:
	HDRFilterComponent();
	~HDRFilterComponent() {}

	bool initialize() override;

	Component* copy() const override;

	void render(RenderContext* context) override;

public:
	float m_exposure;
	float m_white;
};



class HDRFilter : public IFilter {
	typedef glm::vec3 Piexl;

public:
	HDRFilter(PostProcessingManager* mgr);
	~HDRFilter();
	
	static const std::string sName;

	bool initialize() override;

	void apply(Texture* inputFrame, Texture* outputFrame, const FilterParamGroup* params) override;

	void onRenderSizeChange(float w, float h) override;

protected:
	float calcLogAverageLuminance(Texture* frame);

protected:
	RenderTarget m_outputTarget;
	FrameVector<Piexl, StdFrameAlloc<Piexl>> m_pixels;
	FrameAllocator _frameAlloc;
};