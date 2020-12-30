#pragma once
#include"RendererCore.h"
#include<functional>
#include<memory>


class Buffer;
class ShaderProgram;
class IRenderTechnique;


bool RENDER_TASK_EXECUTOR_INIT();
void RENDER_TASK_EXECUTOR_DEINIT();

class RenderTaskExecutor {
public:
	RenderTaskExecutor(IRenderTechnique* rt);
	virtual ~RenderTaskExecutor();

	virtual bool initialize() { return true; }
	virtual void executeMeshTask(const MeshRenderItem_t& task, ShaderProgram* shader) = 0;
	virtual void release() {};


protected:
	IRenderTechnique* m_renderer;
};


class DepthPassRenderTaskExecutor: public RenderTaskExecutor {
public:
	DepthPassRenderTaskExecutor(IRenderTechnique* rt);
	void executeMeshTask(const MeshRenderItem_t& task, ShaderProgram* shader) override;
};



class UlitPassRenderTaskExecutror : public RenderTaskExecutor {
public:
	UlitPassRenderTaskExecutror(IRenderTechnique* rt);
	void executeMeshTask(const MeshRenderItem_t& renderTask, ShaderProgram* shader) override;
};


class LightPassRenderTaskExecuter : public RenderTaskExecutor {
public:
	LightPassRenderTaskExecuter(IRenderTechnique* rt);
	void executeMeshTask(const MeshRenderItem_t& renderTask, ShaderProgram* shader) override;
};


class GeometryPassRenderTaskExecutor : public RenderTaskExecutor {
public:
	GeometryPassRenderTaskExecutor(IRenderTechnique* rt);
	void executeMeshTask(const MeshRenderItem_t& renderTask, ShaderProgram* shader) override;
};


class ShadowPassRenderTaskExecutor : public RenderTaskExecutor {
public:
	ShadowPassRenderTaskExecutor(IRenderTechnique* rt);

	void executeMeshTask(const MeshRenderItem_t& renderTask, ShaderProgram* shader) override;
};


class AmbientPassRenderTaskExecutor : public RenderTaskExecutor {
public:
	AmbientPassRenderTaskExecutor(IRenderTechnique* rt);
	void executeMeshTask(const MeshRenderItem_t& task, ShaderProgram* shader) override;
};



class LightAccumulationPassRenderTaskExecutor : public RenderTaskExecutor {
public:
	LightAccumulationPassRenderTaskExecutor(IRenderTechnique* rt);
	void executeMeshTask(const MeshRenderItem_t& task, ShaderProgram* shader) override;
};