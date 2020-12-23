#pragma once
#include"RendererCore.h"
#include<functional>
#include<memory>


class Buffer;
class ShaderProgram;
class IRenderTechnique;


struct  MaterialBlock {
	glm::vec4 diffuseFactor;
	glm::vec4 specularFactor;
	glm::vec3 emissiveColor;
};


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

	bool initialize() override;
	void executeMeshTask(const MeshRenderItem_t& renderTask, ShaderProgram* shader) override;
	void release() override;

private:
	std::unique_ptr<Buffer> m_materialUBO;
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