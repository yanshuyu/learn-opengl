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
#include"PointLightShadowMapping.h"
#include<sstream>



const std::string DeferredRenderer::s_identifier = "DeferredRenderer";


DeferredRenderer::DeferredRenderer(Renderer* invoker, const RenderingSettings_t& settings): RenderTechnique(invoker)
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
, m_currentPass(RenderPass::None)
, m_directionalLightUBO(nullptr)
, m_pointLightUBO(nullptr)
, m_spotLightUBO(nullptr)
, m_spotLightShadow(nullptr)
, m_dirLightShadow(nullptr)
, m_pointLightShadow(nullptr) {

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

	auto renderSetting = m_invoker->getRenderingSettings();
	// shadow mapping
	m_spotLightShadow.reset(new SpotLightShadowMapping(m_invoker, renderSetting->shadowMapResolution));
	ok = m_spotLightShadow->initialize();
#ifdef _DEBUG
	ASSERT(ok);
#endif // _DEBUG
	if (!ok)
		return false;

	m_dirLightShadow.reset(new DirectionalLightShadowMapping(m_invoker, renderSetting->shadowMapResolution, {0.3f, 0.5f}));
	ok = m_dirLightShadow->initialize();
	if (!ok) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return false;
	}

	m_pointLightShadow.reset(new PointLightShadowMapping(m_invoker, renderSetting->shadowMapResolution));
	ok = m_pointLightShadow->initialize();
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

	m_invoker->setCullFaceMode(CullFaceMode::Back);
	m_invoker->setFaceWindingOrder(FaceWindingOrder::CCW);

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
	m_pointLightShadow.release();
}

bool DeferredRenderer::shouldRunPass(RenderPass pass) {
	return true;
}

void DeferredRenderer::clearScreen(int flags) {
	GLCALL(glClear(flags));
}

void DeferredRenderer::beginFrame() {
	FrameBuffer::bindDefault();
	clearScreen(ClearFlags::Color | ClearFlags::Depth);

	m_gBuffersFBO->bind();
	clearScreen(ClearFlags::Color | ClearFlags::Depth);
}

void DeferredRenderer::endFrame() {
	for (size_t unit = size_t(Texture::Unit::Defualt); unit < size_t(Texture::Unit::MaxUnit); unit++) {
		GLCALL(glActiveTexture(GL_TEXTURE0 + unit));
		GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
	}

	GLCALL(glBindVertexArray(0));

	m_invoker->setDepthTestMode(DepthTestMode::Enable);
	m_invoker->setColorMask(true);

	if (m_activeShader) {
		m_activeShader->unbind();
		m_activeShader = nullptr;
	}
}

void DeferredRenderer::beginDepthPass() {
	m_invoker->setDepthTestMode(DepthTestMode::Enable);
	m_invoker->setDepthTestFunc(DepthFunc::Less);
	m_invoker->setColorMask(false);

	auto shaderMgr = ShaderProgramManager::getInstance();
	auto preZShader = shaderMgr->getProgram("DepthPass");
	if (preZShader.expired())
		preZShader = shaderMgr->addProgram("res/shader/DepthPass.shader");
	ASSERT(!preZShader.expired());

	m_activeShader = preZShader.lock();
	m_activeShader->bind();
	m_currentPass = RenderPass::DepthPass;

	// set view project matrix
	if (m_activeShader->hasUniform("u_VPMat")) {
		auto& camera = m_invoker->getSceneRenderInfo()->camera;
		glm::mat4 vp = camera.projMatrix * camera.viewMatrix;
		m_activeShader->setUniformMat4v("u_VPMat", &vp[0][0]);
	}

	m_invoker->pullingRenderTask();
}

void DeferredRenderer::endDepthPass() {
	m_invoker->setDepthTestMode(DepthTestMode::ReadOnly);
	m_invoker->setDepthTestFunc(DepthFunc::LEqual);
	m_invoker->setColorMask(true);

	if (m_activeShader) {
		m_activeShader->unbind();
		m_activeShader = nullptr;
	}
	m_currentPass = RenderPass::None;
}

void DeferredRenderer::beginGeometryPass() {
	auto geometryShader = ShaderProgramManager::getInstance()->getProgram("GeometryPass");
	if (geometryShader.expired())
		geometryShader = ShaderProgramManager::getInstance()->addProgram("res/shader/GeometryPass.shader");
	ASSERT(!geometryShader.expired());

	m_activeShader = geometryShader.lock();
	m_activeShader->bind();
	m_currentPass = RenderPass::GeometryPass;

	// set view project matrix
	if (m_activeShader->hasUniform("u_VPMat")) {
		auto& camera = m_invoker->getSceneRenderInfo()->camera;
		glm::mat4 vp = camera.projMatrix * camera.viewMatrix;
		m_activeShader->setUniformMat4v("u_VPMat", &vp[0][0]);
	}

	m_invoker->pullingRenderTask();
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
	if (unlitShader.expired())
		unlitShader = ShaderProgramManager::getInstance()->addProgram("res/shader/UnlitDeferred.shader");
	ASSERT(!unlitShader.expired());

	m_activeShader = unlitShader.lock();
	m_activeShader->bind();
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
	m_invoker->setDepthTestMode(DepthTestMode::Enable);
	m_invoker->setDepthTestFunc(DepthFunc::Less);
	
	m_currentPass = RenderPass::ShadowPass;
	auto sceneInfo = m_invoker->getSceneRenderInfo();

	switch (l.type)
	{
	case LightType::SpotLight:
		m_spotLightShadow->beginShadowPhase(l, sceneInfo->camera);
		break;

	case LightType::DirectioanalLight:
		m_dirLightShadow->beginShadowPhase(l, sceneInfo->camera);
		break;

	case LightType::PointLight:
		m_pointLightShadow->beginShadowPhase(l, sceneInfo->camera);
		break;
	
	default:
		break;
	}
}

void DeferredRenderer::endShadowPass(const Light_t& l) {
	switch (l.type)
	{
	case LightType::SpotLight:
		m_spotLightShadow->endShadowPhase(l);
		break;

	case LightType::DirectioanalLight:
		m_dirLightShadow->endShadowPhase(l);
		break;
	
	case LightType::PointLight:
		m_pointLightShadow->endShadowPhase(l);

	default:
		break;
	}

	m_invoker->setDepthTestMode(DepthTestMode::ReadOnly);
	m_invoker->setDepthTestFunc(DepthFunc::LEqual);
	FrameBuffer::bindDefault();

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
			if (directionalLightShader.expired())
				directionalLightShader = ShaderProgramManager::getInstance()->addProgram("res/shader/DirectionalLightDeferred.shader");
			ASSERT(!directionalLightShader.expired());

			m_activeShader = directionalLightShader.lock();
			m_activeShader->bind();
			
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

			auto sceneInfo = m_invoker->getSceneRenderInfo();
			if (m_activeShader->hasUniform("u_VPMat")) {
				glm::mat4 vp = sceneInfo->camera.projMatrix * sceneInfo->camera.viewMatrix;
				m_activeShader->setUniformMat4v("u_VPMat", &vp[0][0]);
			}

			m_dirLightShadow->beginLighttingPhase(l, m_activeShader.get());

		} break;

	case LightType::PointLight: {
			auto pointLightShader = ShaderProgramManager::getInstance()->getProgram("PointLightDeferred");
			if (pointLightShader.expired())
				pointLightShader = ShaderProgramManager::getInstance()->addProgram("res/shader/PointLightDeferred.shader");
			ASSERT(!pointLightShader.expired());

			m_activeShader = pointLightShader.lock();
			m_activeShader->bind();

			// set point light block
			if (m_activeShader->hasUniformBlock("LightBlock")) {
				static PointLightBlock plb;
				plb.position = glm::vec4(l.position, l.range);
				plb.color = glm::vec4(l.color, l.intensity);

				m_pointLightUBO->bind(Buffer::Target::UniformBuffer);
				m_pointLightUBO->loadSubData(&plb, 0, sizeof(plb));
				m_pointLightUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::LightBlock));
				m_activeShader->bindUniformBlock("LightBlock", ShaderProgram::UniformBlockBindingPoint::LightBlock);
			}

			m_pointLightShadow->beginLighttingPhase(l, m_activeShader.get());

		}break;

	case LightType::SpotLight: {
			auto spotLightShader = ShaderProgramManager::getInstance()->getProgram("SpotLightDeferred");
			if (spotLightShader.expired())
				spotLightShader = ShaderProgramManager::getInstance()->addProgram("res/shader/SpotLightDeferred.shader");
			ASSERT(!spotLightShader.expired());

			m_activeShader = spotLightShader.lock();
			m_activeShader->bind();

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
				m_activeShader->bindUniformBlock("LightBlock", ShaderProgram::UniformBlockBindingPoint::LightBlock);
			}

			m_spotLightShadow->beginLighttingPhase(l, m_activeShader.get());

		}break;

	default:
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return;
	}
	
	m_invoker->setBlendMode(BlendMode::Enable);
	m_invoker->setBlendFactor(BlendFactor::One, BlendFactor::One);
	m_invoker->setBlendFunc(BlendFunc::Add);

	m_currentPass = RenderPass::LightPass;

	// set camera position
	if (m_activeShader->hasUniform("u_cameraPosW")) {
		glm::vec3 camPos = m_invoker->getSceneRenderInfo()->camera.position;
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
		m_dirLightShadow->endLighttingPhase(l, m_activeShader.get());

	} else if (l.type == LightType::PointLight) {
		m_pointLightUBO->unbind();
		m_pointLightShadow->endLighttingPhase(l, m_activeShader.get());

	} else if (l.type == LightType::SpotLight) {
		m_spotLightUBO->unbind();
		m_spotLightShadow->endLighttingPhase(l, m_activeShader.get());
	}
	
	m_posWBuffer->unbind();
	m_normalWBuffer->unbind();
	m_diffuseBuffer->unbind();
	m_specularBuffer->unbind();
	m_emissiveBuffer->unbind();

	m_currentPass = RenderPass::None;
	m_invoker->setBlendMode(BlendMode::Disable);

	if (m_activeShader) {
		m_activeShader->unbindUniformBlock("LightBlock");
		m_activeShader->unbind();
		m_activeShader = nullptr;
	}
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

	taskExecutor->second->executeTask(task, m_activeShader.get());
}


void DeferredRenderer::onWindowResize(float w, float h) {
	if (w <= 0 || h <= 0)
		return;
	setupGBuffers();
}


void DeferredRenderer::onShadowMapResolutionChange(float w, float h) {
	m_spotLightShadow->onShadowMapResolutionChange(w, h);
	m_dirLightShadow->onShadowMapResolutionChange(w, h);
	m_pointLightShadow->onShadowMapResolutionChange(w, h);
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
	auto renderSetting = m_invoker->getRenderingSettings();
	success = m_posWBuffer->loadImage2DFromMemory(Texture::Format::RGBA16F, 
													Texture::Format::RGBA,
													Texture::FormatDataType::Float, 
													renderSetting->renderSize.x,
													renderSetting->renderSize.y,
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
														renderSetting->renderSize.x,
														renderSetting->renderSize.y,
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
														renderSetting->renderSize.x,
														renderSetting->renderSize.y,
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
														renderSetting->renderSize.x,
														renderSetting->renderSize.y,
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
														renderSetting->renderSize.x,
														renderSetting->renderSize.y,
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
															renderSetting->renderSize.x,
															renderSetting->renderSize.y,
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

	vertLayoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 3, 0);
	vertLayoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 2, 1);

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