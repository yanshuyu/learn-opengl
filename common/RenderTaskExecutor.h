#pragma once
#include"RendererCore.h"
#include<functional>
#include<memory>


class Buffer;
class ShaderProgram;
class RenderTechnique;


struct  MaterialBlock {
	glm::vec4 diffuseFactor;
	glm::vec4 specularFactor;
	glm::vec3 emissiveColor;
};


class RenderTaskExecutor {

public:
	RenderTaskExecutor(RenderTechnique* rt);
	virtual ~RenderTaskExecutor();

	virtual bool initialize() { return true; }
	virtual void executeTask(const RenderTask_t& renderTask) = 0;
	virtual void release() {};


protected:
	RenderTechnique* m_renderer;
};


class DepthPassRenderTaskExecutor: public RenderTaskExecutor {
public:
	DepthPassRenderTaskExecutor(RenderTechnique* rt);
	void executeTask(const RenderTask_t& renderTask) override;
};



class UlitPassRenderTaskExecutror : public RenderTaskExecutor {
public:
	UlitPassRenderTaskExecutror(RenderTechnique* rt);
	void executeTask(const RenderTask_t& renderTask) override;
};


class LightPassRenderTaskExecuter : public RenderTaskExecutor {
public:
	LightPassRenderTaskExecuter(RenderTechnique* rt);
	bool initialize() override;
	void executeTask(const RenderTask_t& renderTask) override;
	void release() override;

private:
	std::unique_ptr<Buffer> m_materialUBO;
};


class GeometryPassRenderTaskExecutor : public RenderTaskExecutor {
public:
	GeometryPassRenderTaskExecutor(RenderTechnique* rt);

	bool initialize() override;
	void executeTask(const RenderTask_t& renderTask) override;
	void release() override;

private:
	std::unique_ptr<Buffer> m_materialUBO;
};


class ShadowPassRenderTaskExecutor : public RenderTaskExecutor {
public:
	ShadowPassRenderTaskExecutor(RenderTechnique* rt);

	void executeTask(const RenderTask_t& renderTask) override;
};