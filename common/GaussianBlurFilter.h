#pragma once
#include"IFilter.h"
#include"RenderTarget.h"


class GaussianBlurFilterComponent : public FilterComponent {
public:
	GaussianBlurFilterComponent();

	RTTI_DECLARATION(GaussianBlurFilterComponent)

	static const double sMinSigma;
	static const double sMaxSigma;
	static const int sMinKernel;
	static const int sMaxKernel;

	bool initialize() override;

	void render(RenderContext* context) override;
	
	Component* copy() const override { return nullptr; }

	inline void setsigma(float sigma) {
		m_sigma = sigma;
	}

	inline float getSigma() const {
		return m_sigma;
	}

	inline void setKernelSize(int ks) {
		m_kernelSize = ks;
	}

	inline int getKernelSize() const {
		return m_kernelSize;
	}

protected:
	float m_sigma;
	int m_kernelSize;
};




class GaussianBlurFilter : public IFilter {
public:
	GaussianBlurFilter(PostProcessingManager* mgr);

	static const std::string sName;

	void apply(Texture* inputFrame, Texture* outputFrame, const FilterParamGroup* params) override;

protected:
	float gaussian(float sigma, int x);
	void calcWeights(float sigma, int kernel);

protected:
	int m_kernel;
	float m_sigma;
	std::vector<float> m_weights;
	RenderTarget m_outputTarget;
	RenderTarget m_halfBlurTarget;
};