#pragma once
#include"RenderTechnique.h"
#include"RenderTaskExecutor.h"
#include<memory>
#include<unordered_map>


class VertexArray;
class Buffer;
class FrameBuffer;
class Texture;
class ShaderProgram;


class DeferredRenderer : public RenderTechnique {
	struct Vertex {
		glm::vec3 position;
		glm::vec2 uv;

		Vertex(const glm::vec3& _position, const glm::vec2& _uv) {
			position = _position;
			uv = _uv;
		}
	};

	friend class ZPassRenderTaskExecutor;
	friend class GeometryPassRenderTaskExecutor;
	friend class UlitPassRenderTaskExecutror;
	
public:
	DeferredRenderer(size_t renderWidth, size_t renderHeight);
	~DeferredRenderer();

	bool intialize() override;
	void cleanUp() override;

	static const std::string s_identifier;

	void prepareForSceneRenderInfo(const SceneRenderInfo_t* si) override;
	bool shouldVisitScene() const override;

	void clearScrren(int flags) override;

	void beginFrame() override;
	void endFrame() override;

	void beginDepthPass() override;;
	void endDepthPass() override;

	void beginGeometryPass() override;
	void endGeometryPass() override;

	void beginUnlitPass() override;
	void endUnlitPass() override;

	void beginLightPass(const Light_t& l) override;
	void endLightPass(const Light_t& l) override;

	void beginTransparencyPass() override;
	void endTransparencyPass() override;

	void onWindowResize(float w, float h) override;
	void performTask(const RenderTask_t& task)  override;


	RenderPass currentRenderPass() const override {
		return m_currentPass;
	}

	std::string identifier() const override {
		return s_identifier;
	}

private:
	bool setupGBuffers();
	void setupFullScreenQuad();
	void drawFullScreenQuad();

	void beginDirectionalLightPass(const Light_t& l);
	void endDirectionalLightPass();

	void beginPointLightPass(const Light_t& l);
	void endPointLightPass();

	void beginSpotLightPass(const Light_t& l);
	void endSpotLightPass();

private:
	size_t m_renderWidth;
	size_t m_renderHeight;
	const SceneRenderInfo_t*  m_sceneInfo;
	ShaderProgram* m_activeShader;
	RenderPass m_currentPass;
	std::unordered_map<RenderPass, std::unique_ptr<RenderTaskExecutor>> m_taskExecutors;

	// geometry buffers
	std::unique_ptr<FrameBuffer> m_gBuffersFBO;
	std::unique_ptr<Texture> m_posWBuffer;
	std::unique_ptr<Texture> m_normalWBuffer;
	std::unique_ptr<Texture> m_diffuseBuffer;
	std::unique_ptr<Texture> m_specularBuffer;
	std::unique_ptr<Texture> m_emissiveBuffer;
	std::unique_ptr<Texture> m_depthStencilBuffer;

	// screen quad
	std::unique_ptr<VertexArray> m_quadVAO;
	std::unique_ptr<Buffer> m_quadVBO;
	std::unique_ptr<Buffer> m_quadIBO;

	// light ubo
	std::unique_ptr<Buffer> m_directionalLightUBO;
	std::unique_ptr<Buffer> m_pointLightUBO;
	std::unique_ptr<Buffer> m_spotLightUBO;
};



