#pragma once
#include"IFilter.h"
#include"RenderTarget.h"
#include"Buffer.h"
#include"Texture.h"

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

protected:
	RenderTarget m_outputTarget;
	Buffer m_histBuffer;
	Buffer m_atomicCounter;
	Texture m_aveLumTex;
};