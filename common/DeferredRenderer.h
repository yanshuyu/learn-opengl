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

	friend class DepthPassRenderTaskExecutor;
	friend class GeometryPassRenderTaskExecutor;
	friend class UlitPassRenderTaskExecutror;
	friend class ShadowPassRenderTaskExecutor;
	
public:
	DeferredRenderer(const RenderingSettings_t& settings);
	~DeferredRenderer();

	DeferredRenderer(const DeferredRenderer& other) = delete;
	DeferredRenderer(DeferredRenderer&& rv) = delete;
	DeferredRenderer& operator = (const DeferredRenderer& other) = delete;
	DeferredRenderer& operator = (DeferredRenderer&& rv) = delete;

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

	void beginShadowPass(const Light_t& l) override;
	void endShadowPass(const Light_t& l) override;

	void beginLightPass(const Light_t& l) override;
	void endLightPass(const Light_t& l) override;

	void beginTransparencyPass() override;
	void endTransparencyPass() override;

	void onWindowResize(float w, float h) override;
	void onShadowMapResolutionChange(float w, float h) override;

	void performTask(const RenderTask_t& task)  override;


	RenderPass currentRenderPass() const override {
		return m_currentPass;
	}

	std::string identifier() const override {
		return s_identifier;
	}

private:
	bool setupGBuffers();
	bool setupShadowMap();
	void setupFullScreenQuad();
	void drawFullScreenQuad();

private:
	RenderingSettings_t m_renderingSettings;

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

	// shadow mapping
	std::unique_ptr<FrameBuffer> m_shadowMapFBO;
	std::unique_ptr<Texture> m_shadowMap;
	std::unique_ptr<Buffer> m_shadowUBO;
	glm::mat4 m_lightVPMat;
};



