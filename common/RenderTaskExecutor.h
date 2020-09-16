#pragma once
#include"RendererCore.h"
#include"ShaderProgram.h"
#include<functional>


class RenderTaskExecutor {
public:
	enum class RendererType {
		Forward,
		Deferred,
	};

public:
	RenderTaskExecutor(RendererType rt);
	virtual ~RenderTaskExecutor();

	virtual bool initialize() { return true; }
	virtual void executeTask(const RenderTask_t& renderTask, const SceneRenderInfo_t& renderInfo, ShaderProgram* shader) = 0;
	virtual void release() {};

	inline void setRendererType(RendererType rt) {
		m_rendererType = rt;
	}

	inline RendererType getRendererType() const {
		return m_rendererType;
	}

protected:
	RendererType m_rendererType;
};


class ZPassRenderTaskExecutor: public RenderTaskExecutor {
public:
	ZPassRenderTaskExecutor(RendererType rt);
	void executeTask(const RenderTask_t& renderTask, const SceneRenderInfo_t& renderInfo, ShaderProgram* shader) override;
};



class UlitPassRenderTaskExecutror : public RenderTaskExecutor {
public:
	UlitPassRenderTaskExecutror(RendererType rt);
	void executeTask(const RenderTask_t& renderTask, const SceneRenderInfo_t& renderInfo, ShaderProgram* shader) override;
};