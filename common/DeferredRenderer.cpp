#include"DeferredRenderer.h"
#include"ShaderProgamMgr.h"
#include"VertexArray.h"
#include"VertexLayoutDescription.h"
#include"FrameBuffer.h"
#include"Texture.h"
#include"Material.h"
#include"Util.h"
#include"Renderer.h"
#include"SpotLightShadowMapping.h"
#include"DirectionalLightShadowMapping.h"
#include<sstream>



const std::string DeferredRenderer::s_identifier = "DeferredRenderer";


DeferredRenderer::DeferredRenderer(Renderer* invoker, const RenderingSettings_t& settings): RenderTechnique(invoker)
, m_renderingSettings(settings)
, m_sceneInfo(nullptr)
, m_gBuffersFBO(nullptr)
, m_posWBuffer(nullptr)
, m_normalWBuffer(nullptr)
, m_diffuseBuffer(nullptr)
, m_specularBuffer(nullptr)
, m_emissiveBuffer(nullptr)
, m_depthStencilBuffer(nullptr)
, m_quadVAO(nullptr)
, m_quadVBO(nullptr)
, m_quadIBO(nullptr)
, m_activeShader(nullptr)
, m_currentPass(RenderPass::None)
, m_directionalLightUBO(nullptr)
, m_pointLightUBO(nullptr)
, m_spotLightUBO(nullptr)
, m_spotLightShadow(nullptr)
, m_dirLightShadow(nullptr){

}

DeferredRenderer::~DeferredRenderer() {
	cleanUp();
}


bool DeferredRenderer::intialize() {
	bool ok = true;

	// g-buffers
	ok = setupGBuffers();
#ifdef _DEBUG
	ASSERT(ok);
#endif // _DEBUG
	if (!ok)
		return false;

	// shadow mapping
	m_spotLightShadow.reset(new SpotLightShadowMapping(this, 
				{ m_renderingSettings.shadowMapResolution.x, m_renderingSettings.shadowMapResolution.y }));
	ok = m_spotLightShadow->initialize();
#ifdef _DEBUG
	ASSERT(ok);
#endif // _DEBUG
	if (!ok)
		return false;

	m_dirLightShadow.reset(new DirectionalLightShadowMapping(this, m_renderingSettings.shadowMapResolution, {0.3f, 0.5f}));
	ok = m_dirLightShadow->initialize();
	if (!ok) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return false;
	}

	setupFullScreenQuad();

	// light ubo
	m_directionalLightUBO.reset(new Buffer());
	m_directionalLightUBO->bind(Buffer::Target::UniformBuffer);
	m_directionalLightUBO->loadData(nullptr, sizeof(DirectionalLightBlock), Buffer::Usage::StaticDraw);
	m_directionalLightUBO->unbind();

	m_pointLightUBO.reset(new Buffer());
	m_pointLightUBO->bind(Buffer::Target::UniformBuffer);
	m_pointLightUBO->loadData(nullptr, sizeof(PointLightBlock), Buffer::Usage::StaticDraw);
	m_pointLightUBO->unbind();

	m_spotLightUBO.reset(new Buffer());
	m_spotLightUBO->bind(Buffer::Target::UniformBuffer);
	m_spotLightUBO->loadData(nullptr, sizeof(SpotLightBlock), Buffer::Usage::StaticDraw);
	m_spotLightUBO->unbind();

	// render task executor
	m_taskExecutors[RenderPass::DepthPass] = std::unique_ptr<RenderTaskExecutor>(new DepthPassRenderTaskExecutor(this));
	m_taskExecutors[RenderPass::GeometryPass] = std::unique_ptr<RenderTaskExecutor>(new GeometryPassRenderTaskExecutor(this));
	m_taskExecutors[RenderPass::UnlitPass] = std::unique_ptr<RenderTaskExecutor>(new UlitPassRenderTaskExecutror(this));
	m_taskExecutors[RenderPass::LightPass] = std::unique_ptr<RenderTaskExecutor>(new LightPassRenderTaskExecuter(this));
	m_taskExecutors[RenderPass::ShadowPass] = std::unique_ptr<RenderTaskExecutor>(new ShadowPassRenderTaskExecutor(this));

	for (auto& executor : m_taskExecutors) {
		if (!executor.second->initialize()) {
			ok = false;
#ifdef _DEBUG
			ASSERT(false);
#endif // _DEBUG
			break;
		}
	}

	GLCALL(glEnable(GL_CULL_FACE));
	GLCALL(glCullFace(GL_BACK));
	GLCALL(glFrontFace(GL_CCW));

	return ok;
}


void DeferredRenderer::cleanUp() {
	m_posWBuffer.release();
	m_normalWBuffer.release();
	m_diffuseBuffer.release();
	m_specularBuffer.release();
	m_emissiveBuffer.release();
	m_gBuffersFBO.release();

	m_quadVAO.release();
	m_quadVBO.release();
	m_quadIBO.release();

	m_directionalLightUBO.release();
	m_pointLightUBO.release();
	m_spotLightUBO.release();

	m_spotLightShadow.release();
	m_dirLightShadow.release();
}


void DeferredRenderer::prepareForSceneRenderInfo(const SceneRenderInfo_t* si) {
	m_sceneInfo = si;
}


bool DeferredRenderer::shouldRunPass(RenderPass pass) {
	return true;
}

void DeferredRenderer::pullingRenderTask(ShaderProgram* shader) {
	if (shader)
		m_activeShader = shader;

	__super::pullingRenderTask();
}


void DeferredRenderer::clearScrren(int flags) {
	GLCALL(glClear(flags));
}

void DeferredRenderer::beginFrame() {
	if (m_sceneInfo->camera.backgrounColor != m_clearColor)
		setClearColor(m_sceneInfo->camera.backgrounColor);

	if (m_sceneInfo->camera.viewport != m_viewPort)
		setViewPort(m_sceneInfo->camera.viewport);

	// clear default fbo
	FrameBuffer::bindDefault();
	clearScrren(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// clear g-buffers
	m_gBuffersFBO->bind();
	clearScrren(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

void DeferredRenderer::endFrame() {
#ifdef _DEBUG
	/*
	float renderWidth = m_renderingSettings.renderSize.x;
	float renderHeight = m_renderingSettings.renderSize.y;
	m_spotLightShadow->visualizeShadowMap({ renderWidth, renderHeight }, { 0.f, 0.f, renderWidth * 0.25f, renderHeight * 0.25f });
	*/
#endif // _DEBUG

	if (m_activeShader) {
		m_activeShader->unbind();
		m_activeShader = nullptr;
	}


	for (size_t unit = size_t(Texture::Unit::Defualt); unit < size_t(Texture::Unit::MaxUnit); unit++) {
		GLCALL(glActiveTexture(GL_TEXTURE0 + unit));
		GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
	}

	GLCALL(glBindVertexArray(0));
	GLCALL(glDepthMask(GL_TRUE));
	GLCALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
}

void DeferredRenderer::beginDepthPass() {
	GLCALL(glEnable(GL_DEPTH_TEST));
	GLCALL(glDepthFunc(GL_LESS));
	GLCALL(glDepthMask(GL_TRUE));
	GLCALL(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));

	auto shaderMgr = ShaderProgramManager::getInstance();
	auto preZShader = shaderMgr->getProgram("DepthPass");
	if (!preZShader)
		preZShader = shaderMgr->addProgram("res/shader/DepthPass.shader");

	ASSERT(preZShader);

	preZShader->bind();

	m_activeShader = preZShader.get();
	m_currentPass = RenderPass::DepthPass;

	// set view project matrix
	if (m_activeShader->hasUniform("u_VPMat")) {
		auto& camera = m_sceneInfo->camera;
		glm::mat4 vp = camera.projMatrix * camera.viewMatrix;
		m_activeShader->setUniformMat4v("u_VPMat", &vp[0][0]);
	}

	pullingRenderTask();
}

void DeferredRenderer::endDepthPass() {
	GLCALL(glEnable(GL_DEPTH_TEST));
	GLCALL(glDepthFunc(GL_LEQUAL));
	GLCALL(glDepthMask(GL_FALSE));
	GLCALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

	if (m_activeShader) {
		m_activeShader->unbind();
		m_activeShader = nullptr;
	}
	m_currentPass = RenderPass::None;
}

void DeferredRenderer::beginGeometryPass() {
	auto geometryShader = ShaderProgramManager::getInstance()->getProgram("GeometryPass");
	if (!geometryShader)
		geometryShader = ShaderProgramManager::getInstance()->addProgram("res/shader/GeometryPass.shader");
	ASSERT(geometryShader);

	geometryShader->bind();
	m_activeShader = geometryShader.get();
	m_currentPass = RenderPass::GeometryPass;

	// set view project matrix
	if (m_activeShader->hasUniform("u_VPMat")) {
		auto& camera = m_sceneInfo->camera;
		glm::mat4 vp = camera.projMatrix * camera.viewMatrix;
		m_activeShader->setUniformMat4v("u_VPMat", &vp[0][0]);
	}

	pullingRenderTask();
}

void DeferredRenderer::endGeometryPass() {
	m_gBuffersFBO->unbind();
	FrameBuffer::bindDefault();
	
	if (m_activeShader) {
		m_activeShader->unbind();
		m_activeShader = nullptr;
	}
	m_currentPass = RenderPass::None;
}

void DeferredRenderer::beginUnlitPass() {
	auto unlitShader = ShaderProgramManager::getInstance()->getProgram("UnlitDeferred");
	if (!unlitShader)
		unlitShader = ShaderProgramManager::getInstance()->addProgram("res/shader/UnlitDeferred.shader");
	ASSERT(unlitShader);

	unlitShader->bind();
	m_activeShader = unlitShader.get();
	m_currentPass = RenderPass::UnlitPass;

	// manually sumit render task, draw a full screen quad
	drawFullScreenQuad();
}

void DeferredRenderer::endUnlitPass() {
	m_diffuseBuffer->unbind();
	m_emissiveBuffer->unbind();
	if (m_activeShader) {
		m_activeShader->unbind();
		m_activeShader = nullptr;
	}
	m_currentPass = RenderPass::None;
}

void DeferredRenderer::beginShadowPass(const Light_t& l) {
	//restore depth buffer writable
	GLCALL(glDepthFunc(GL_LESS));
	GLCALL(glDepthMask(GL_TRUE));
	
	m_currentPass = RenderPass::ShadowPass;

	switch (l.type)
	{
	case LightType::SpotLight:
		m_spotLightShadow->beginShadowPhase(l, m_sceneInfo->camera);
		break;

	case LightType::DirectioanalLight:
		m_dirLightShadow->beginShadowPhase(l, m_sceneInfo->camera);
		break;

	default:
		break;
	}
}

void DeferredRenderer::endShadowPass(const Light_t& l) {
	switch (l.type)
	{
	case LightType::SpotLight:
		m_spotLightShadow->endShadowPhase(l, m_sceneInfo->camera);
		break;

	case LightType::DirectioanalLight:
		m_dirLightShadow->endShadowPhase(l, m_sceneInfo->camera);
		break;

	default:
		break;
	}

	GLCALL(glDepthFunc(GL_LEQUAL));
	GLCALL(glDepthMask(GL_FALSE));

	if (m_activeShader) {
		m_activeShader->unbind();
		m_activeShader = nullptr;
	}
	m_currentPass = RenderPass::None;
}

void DeferredRenderer::beginLightPass(const Light_t& l) {
	switch (l.type)
	{
	case LightType::DirectioanalLight: {
			auto directionalLightShader = ShaderProgramManager::getInstance()->getProgram("DirectionalLightDeferred");
			if (!directionalLightShader)
				directionalLightShader = ShaderProgramManager::getInstance()->addProgram("res/shader/DirectionalLightDeferred.shader");
			ASSERT(directionalLightShader);

			directionalLightShader->bind();
			m_activeShader = directionalLightShader.get();

			// set directional light block
			if (m_activeShader->hasUniformBlock("LightBlock")) {
				static DirectionalLightBlock dlb;
				dlb.color = glm::vec4(glm::vec3(l.color), l.intensity);
				dlb.inverseDiretion = -l.direction;
				m_directionalLightUBO->bind(Buffer::Target::UniformBuffer);
				m_directionalLightUBO->loadSubData(&dlb, 0, sizeof(dlb));

				m_directionalLightUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::LightBlock));
				m_activeShader->bindUniformBlock("LightBlock", ShaderProgram::UniformBlockBindingPoint::LightBlock);
			}

			if (m_activeShader->hasUniform("u_VPMat")) {
				glm::mat4 vp = m_sceneInfo->camera.projMatrix * m_sceneInfo->camera.viewMatrix;
				m_activeShader->setUniformMat4v("u_VPMat", &vp[0][0]);
			}

			m_dirLightShadow->beginLighttingPhase(l, m_activeShader);

		} break;

	case LightType::PointLight: {
			auto pointLightShader = ShaderProgramManager::getInstance()->getProgram("PointLightDeferred");
			if (!pointLightShader)
				pointLightShader = ShaderProgramManager::getInstance()->addProgram("res/shader/PointLightDeferred.shader");
			ASSERT(pointLightShader);

			pointLightShader->bind();
			m_activeShader = pointLightShader.get();

			// set point light block
			if (m_activeShader->hasUniformBlock("LightBlock")) {
				static PointLightBlock plb;
				plb.position = glm::vec4(l.position, l.range);
				plb.color = glm::vec4(l.color, l.intensity);

				m_pointLightUBO->bind(Buffer::Target::UniformBuffer);
				m_pointLightUBO->loadSubData(&plb, 0, sizeof(plb));
				m_pointLightUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::LightBlock));
				pointLightShader->bindUniformBlock("LightBlock", ShaderProgram::UniformBlockBindingPoint::LightBlock);
			}

		}break;

	case LightType::SpotLight: {
			auto spotLightShader = ShaderProgramManager::getInstance()->getProgram("SpotLightDeferred");
			if (!spotLightShader)
				spotLightShader = ShaderProgramManager::getInstance()->addProgram("res/shader/SpotLightDeferred.shader");
			ASSERT(spotLightShader);

			spotLightShader->bind();
			m_activeShader = spotLightShader.get();

			// set spot light block
			if (m_activeShader->hasUniformBlock("LightBlock")) {
				static SpotLightBlock slb;
				slb.position = glm::vec4(l.position, l.range);
				slb.color = glm::vec4(l.color, l.intensity);
				slb.inverseDirection = -l.direction;
				slb.angles = glm::vec2(l.innerCone, l.outterCone);

				m_spotLightUBO->bind(Buffer::Target::UniformBuffer);
				m_spotLightUBO->loadSubData(&slb, 0, sizeof(slb));
				m_spotLightUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::LightBlock));
				spotLightShader->bindUniformBlock("LightBlock", ShaderProgram::UniformBlockBindingPoint::LightBlock);
			}

			m_spotLightShadow->beginLighttingPhase(l, m_activeShader);

		}break;

	default:
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return;
	}
	
	GLCALL(glEnable(GL_BLEND));
	GLCALL(glBlendFunc(GL_ONE, GL_ONE));
	GLCALL(glBlendEquation(GL_FUNC_ADD));

	m_currentPass = RenderPass::LightPass;

	// set camera position
	if (m_activeShader->hasUniform("u_cameraPosW")) {
		glm::vec3 camPos = m_sceneInfo->camera.position;
		m_activeShader->setUniform3v("u_cameraPosW", &camPos[0]);
	}

	// set max shininess
	if (m_activeShader->hasUniform("u_maxShininess")) {
		m_activeShader->setUniform1("u_maxShininess", float(Material::s_maxShininess));
	}


	// set g-buffers
	if (m_activeShader->hasUniform("u_posW")) {
		m_posWBuffer->bind(Texture::Unit::Position);
		m_activeShader->setUniform1("u_posW", int(Texture::Unit::Position));
	}

	if (m_activeShader->hasUniform("u_nromalW")) {
		m_normalWBuffer->bind(Texture::Unit::NormalMap);
		m_activeShader->setUniform1("u_nromalW", int(Texture::Unit::NormalMap));
	}

	if (m_activeShader->hasUniform("u_diffuse")) {
		m_diffuseBuffer->bind(Texture::Unit::DiffuseMap);
		m_activeShader->setUniform1("u_diffuse", int(Texture::Unit::DiffuseMap));
	}

	if (m_activeShader->hasUniform("u_specular")) {
		m_specularBuffer->bind(Texture::Unit::SpecularMap);
		m_activeShader->setUniform1("u_specular", int(Texture::Unit::SpecularMap));
	}

	if (m_activeShader->hasUniform("u_emissive")) {
		m_emissiveBuffer->bind(Texture::Unit::EmissiveMap);
		m_activeShader->setUniform1("u_emissive", int(Texture::Unit::EmissiveMap));
	}

	// manully submit render task
	drawFullScreenQuad();
}

void DeferredRenderer::endLightPass(const Light_t& l) {
	if (l.type == LightType::DirectioanalLight) {
		m_directionalLightUBO->unbind();
		m_dirLightShadow->endLighttingPhase(l, m_activeShader);

	} else if (l.type == LightType::PointLight) {
		m_pointLightUBO->unbind();

	} else if (l.type == LightType::SpotLight) {
		m_spotLightUBO->unbind();
		m_spotLightShadow->endLighttingPhase(l, m_activeShader);
	}

	if (m_activeShader) {
		m_activeShader->unbindUniformBlock("LightBlock");
		m_activeShader->unbind();
		m_activeShader = nullptr;
	}
	
	m_posWBuffer->unbind();
	m_normalWBuffer->unbind();
	m_diffuseBuffer->unbind();
	m_specularBuffer->unbind();
	m_emissiveBuffer->unbind();


	m_currentPass = RenderPass::None;
	
	GLCALL(glDisable(GL_BLEND));
	GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

void DeferredRenderer::beginTransparencyPass() {

}

void DeferredRenderer::endTransparencyPass() {

}

void DeferredRenderer::performTask(const RenderTask_t& task) {
	auto taskExecutor = m_taskExecutors.find(m_currentPass);
	if (taskExecutor == m_taskExecutors.end()) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		std::stringstream msg;
		msg << "renderer no task executor for pass: " << int(m_currentPass) << "\n";
		CONSOLELOG(msg.str());
		return;
	}

	taskExecutor->second->executeTask(task, m_activeShader);
}


void DeferredRenderer::onWindowResize(float w, float h) {
	if (w <= 0 || h <= 0)
		return;
	m_renderingSettings.renderSize.x = w;
	m_renderingSettings.renderSize.y = h;
	setupGBuffers();
}


void DeferredRenderer::onShadowMapResolutionChange(float w, float h) {
	m_renderingSettings.shadowMapResolution = { w, h };
	m_spotLightShadow->onShadowMapResolutionChange(w, h);
	m_dirLightShadow->onShadowMapResolutionChange(w, h);
}


bool DeferredRenderer::setupGBuffers() {
	bool success = true;
	m_gBuffersFBO.reset(new FrameBuffer());
	m_posWBuffer.reset(new Texture());
	m_normalWBuffer.reset(new Texture());
	m_diffuseBuffer.reset(new Texture());
	m_specularBuffer.reset(new Texture());
	m_emissiveBuffer.reset(new Texture());
	m_depthStencilBuffer.reset(new Texture());

	m_posWBuffer->bind();
	success = m_posWBuffer->loadImage2DFromMemory(Texture::Format::RGBA16F, 
													Texture::Format::RGBA,
													Texture::FormatDataType::Float, 
													m_renderingSettings.renderSize.x,
													m_renderingSettings.renderSize.y, 
													nullptr);
	m_posWBuffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
	m_posWBuffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
	m_posWBuffer->unbind();

	if (!success)
		return false;

	m_normalWBuffer->bind();
	success = m_normalWBuffer->loadImage2DFromMemory(Texture::Format::RGBA16F,
														Texture::Format::RGBA,
														Texture::FormatDataType::Float, 
														m_renderingSettings.renderSize.x,
														m_renderingSettings.renderSize.y, 
														nullptr);
	m_normalWBuffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
	m_normalWBuffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
	m_normalWBuffer->unbind();

	if (!success)
		return false;

	m_diffuseBuffer->bind();
	success = m_diffuseBuffer->loadImage2DFromMemory(Texture::Format::RGBA,
														Texture::Format::RGBA,
														Texture::FormatDataType::UByte,
														m_renderingSettings.renderSize.x,
														m_renderingSettings.renderSize.y,
														nullptr);
	m_diffuseBuffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
	m_diffuseBuffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
	m_diffuseBuffer->unbind();

	if (!success)
		return false;

	m_specularBuffer->bind();
	success = m_specularBuffer->loadImage2DFromMemory(Texture::Format::RGBA,
		Texture::Format::RGBA, 
		Texture::FormatDataType::UByte,
		m_renderingSettings.renderSize.x,
		m_renderingSettings.renderSize.y,
		nullptr);
	m_specularBuffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
	m_specularBuffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
	m_specularBuffer->unbind();

	if (!success)
		return false;

	m_emissiveBuffer->bind();
	success = m_emissiveBuffer->loadImage2DFromMemory(Texture::Format::RGBA,
		Texture::Format::RGBA,
		Texture::FormatDataType::UByte,
		m_renderingSettings.renderSize.x,
		m_renderingSettings.renderSize.y,
		nullptr);
	m_emissiveBuffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
	m_emissiveBuffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
	m_emissiveBuffer->unbind();

	if (!success)
		return false;

	m_depthStencilBuffer->bind();
	success = m_depthStencilBuffer->loadImage2DFromMemory(Texture::Format::Depth24_Stencil8,
		Texture::Format::Depth_Stencil,
		Texture::FormatDataType::UInt_24_8,
		m_renderingSettings.renderSize.x,
		m_renderingSettings.renderSize.y,
		nullptr);
	m_depthStencilBuffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
	m_depthStencilBuffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
	m_depthStencilBuffer->unbind();

	if (!success)
		return false;

	m_gBuffersFBO->bind();
	m_gBuffersFBO->addTextureAttachment(m_posWBuffer->getHandler(), FrameBuffer::AttachmentPoint::Color, 0);
	m_gBuffersFBO->addTextureAttachment(m_normalWBuffer->getHandler(), FrameBuffer::AttachmentPoint::Color, 1);
	m_gBuffersFBO->addTextureAttachment(m_diffuseBuffer->getHandler(), FrameBuffer::AttachmentPoint::Color, 2);
	m_gBuffersFBO->addTextureAttachment(m_specularBuffer->getHandler(), FrameBuffer::AttachmentPoint::Color, 3);
	m_gBuffersFBO->addTextureAttachment(m_emissiveBuffer->getHandler(), FrameBuffer::AttachmentPoint::Color, 4);
	m_gBuffersFBO->addTextureAttachment(m_depthStencilBuffer->getHandler(), FrameBuffer::AttachmentPoint::Depth_Stencil);
	m_gBuffersFBO->setDrawBufferLocation({ 0, 1, 2, 3, 4 });
	FrameBuffer::Status status = m_gBuffersFBO->checkStatus();
	m_gBuffersFBO->unbind();

	return  status == FrameBuffer::Status::Ok;
}

void DeferredRenderer::setupFullScreenQuad() {
	Vertex quadVertices[] = {
		Vertex({-1.f, -1.f, 0.f}, {0.f, 0.f}),
		Vertex({1.f, -1.f, 0.f}, {1.f, 0.f}),
		Vertex({1.f, 1.f, 0.f}, {1.f, 1.f}),
		Vertex({-1.f, 1.f, 0.f}, {0.f, 1.f}),
	};

	unsigned int quadIndices[] = {
		0,1,2,
		2,3,0
	};

	VertexLayoutDescription vertLayoutDesc;
	m_quadVAO.reset(new VertexArray());
	m_quadVBO.reset(new Buffer());
	m_quadIBO.reset(new Buffer());

	m_quadVAO->bind();
	m_quadVBO->bind(Buffer::Target::VertexBuffer);
	m_quadIBO->bind(Buffer::Target::IndexBuffer);
	m_quadVBO->loadData(&quadVertices[0], sizeof(Vertex) * 4, Buffer::Usage::StaticDraw, 4);
	m_quadIBO->loadData(&quadIndices[0], sizeof(unsigned int) * 6, Buffer::Usage::StaticDraw, 6);

	vertLayoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 3);
	vertLayoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 2);

	m_quadVAO->storeVertexLayout(vertLayoutDesc);
	
	m_quadVAO->unbind();
	m_quadVBO->unbind();
	m_quadIBO->unbind();
}


void DeferredRenderer::drawFullScreenQuad() {
	RenderTask_t task;
	task.vao = m_quadVAO.get();
	task.indexCount = m_quadIBO->getElementCount();
	task.primitive = PrimitiveType::Triangle;
	performTask(task);
}