#pragma once
#include"IFilter.h"
#include"RenderTarget.h"


class HDRFilterComponent2 : public FilterComponent {
public:
	HDRFilterComponent2();

	RTTI_DECLARATION(HDRFilterComponent2)

	bool initialize() override;

	Component* copy() const override { return nullptr; }

	void render(RenderContext* context) override;

	inline float getExposure() const {
		return m_exposure;
	}

	inline void setExposure(float exposure) {
		m_exposure = exposure;
	}

protected:
	float m_exposure;
};




class PostProcessingManager;

class HDRFilter2 : public IFilter {
public:
	HDRFilter2(PostProcessingManager* mgr);

	static const std::string sName;

	void apply(Texture* inputFrame, Texture* outputFrame, const FilterParamGroup* params) override;

protected:
	RenderTarget m_outputTarget;
};



