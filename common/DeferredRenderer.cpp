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


DeferredRenderer::DeferredRenderer(Renderer* renderer): RenderTechniqueBase(renderer)
, m_gBuffersFBO(nullptr)
, m_posWBuffer(nullptr)
, m_normalWBuffer(nullptr)
, m_diffuseBuffer(nullptr)
, m_specularBuffer(nullptr)
, m_emissiveBuffer(nullptr)
, m_depthStencilBuffer(nullptr)
, m_directionalLightUBO(nullptr)
, m_pointLightUBO(nullptr)
, m_spotLightUBO(nullptr)
, m_depthPassPipelineState()
, m_geometryPassPipelineState()
, m_shadowPassPipelineState()
, m_lightPassPipelineState()
, m_unlitPassPipelineState()
, m_shadowMappings() {

}

DeferredRenderer::~DeferredRenderer() {
	cleanUp();
}


bool DeferredRenderer::intialize() {
	m_depthPassPipelineState.depthMode = DepthMode::Enable;
	m_depthPassPipelineState.depthFunc = DepthFunc::Less;
	m_depthPassPipelineState.depthMask = 1;

	m_geometryPassPipelineState.depthMode = DepthMode::Enable;
	m_geometryPassPipelineState.depthFunc = DepthFunc::LEqual;
	m_geometryPassPipelineState.depthMask = 0;

	m_shadowPassPipelineState.depthMode = DepthMode::Enable;
	m_shadowPassPipelineState.depthFunc = DepthFunc::Less;
	m_shadowPassPipelineState.depthMask = 1;
	m_shadowPassPipelineState.cullMode = CullFaceMode::Front;
	m_shadowPassPipelineState.cullFaceWindingOrder = FaceWindingOrder::CCW;

	m_unlitPassPipelineState.depthMode = DepthMode::Disable;

	m_lightPassPipelineState.depthMode = DepthMode::Disable;
	m_lightPassPipelineState.blendMode = BlendMode::Enable;
	m_lightPassPipelineState.blendSrcFactor = BlendFactor::One;
	m_lightPassPipelineState.blendDstFactor = BlendFactor::One;
	m_lightPassPipelineState.blendFunc = BlendFunc::Add;

	bool ok = true;

	// g-buffers
	ok = setupGBuffers();

#ifdef _DEBUG
	ASSERT(ok);
#endif // _DEBUG
	
	if (!ok)
		return false;

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
	
	m_renderer->pushGPUPipelineState(&GPUPipelineState::s_defaultState);
	m_renderer->setColorMask(true);

	return ok;
}


void DeferredRenderer::cleanUp() {
	m_posWBuffer.release();
	m_normalWBuffer.release();
	m_diffuseBuffer.release();
	m_specularBuffer.release();
	m_emissiveBuffer.release();
	m_gBuffersFBO.release();

	m_directionalLightUBO.release();
	m_pointLightUBO.release();
	m_spotLightUBO.release();

	m_shadowMappings.clear();
}


void DeferredRenderer::beginFrame() {
	m_renderer->clearScreen(ClearFlags::Color | ClearFlags::Depth);

	m_renderer->pushRenderTarget(m_gBuffersFBO.get());
	m_renderer->clearScreen(ClearFlags::Color | ClearFlags::Depth);
}

void DeferredRenderer::endFrame() {
	for (size_t unit = size_t(Texture::Unit::Defualt); unit < size_t(Texture::Unit::MaxUnit); unit++) {
		GLCALL(glActiveTexture(GL_TEXTURE0 + unit));
		GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
	}

	GLCALL(glBindVertexArray(0));

	if (m_passShader) {
		m_passShader->unbind();
		m_passShader = nullptr;
	}
}

void DeferredRenderer::drawDepthPass(const Scene_t& scene) {
	m_pass = RenderPass::DepthPass;
	m_renderer->pushGPUPipelineState(&m_depthPassPipelineState);
	m_renderer->setColorMask(false);

	auto shaderMgr = ShaderProgramManager::getInstance();
	auto preZShader = shaderMgr->getProgram("DepthPass");
	if (preZShader.expired())
		preZShader = shaderMgr->addProgram("DepthPass.shader");
	ASSERT(!preZShader.expired());

	m_passShader = preZShader.lock();
	m_renderer->pushShaderProgram(m_passShader.get());

	// set view project matrix
	if (m_passShader->hasUniform("u_VPMat")) {
		auto& camera = *scene.mainCamera;
		glm::mat4 vp = camera.projMatrix * camera.viewMatrix;
		m_passShader->setUniformMat4v("u_VPMat", &vp[0][0]);
	}

	for (size_t i = 0; i < scene.numOpaqueItems; i++) {
		render(scene.opaqueItems[i]);
	}

	m_renderer->popGPUPipelineState();
	m_renderer->setColorMask(true);
	m_renderer->popShadrProgram();
	m_passShader = nullptr;
	m_pass = RenderPass::None;
}


void DeferredRenderer::drawGeometryPass(const Scene_t& scene) {
	m_pass = RenderPass::GeometryPass;
	m_renderer->pushGPUPipelineState(&m_geometryPassPipelineState);

	auto geometryShader = ShaderProgramManager::getInstance()->getProgram("GeometryPass");
	if (geometryShader.expired())
		geometryShader = ShaderProgramManager::getInstance()->addProgram("GeometryPass.shader");
	ASSERT(!geometryShader.expired());

	m_passShader = geometryShader.lock();
	m_renderer->pushShaderProgram(m_passShader.get());

	// set view project matrix
	if (m_passShader->hasUniform("u_VPMat")) {
		auto& camera = *scene.mainCamera;
		glm::mat4 vp = camera.projMatrix * camera.viewMatrix;
		m_passShader->setUniformMat4v("u_VPMat", &vp[0][0]);
	}

	for (size_t i = 0; i < scene.numOpaqueItems; i++) {
		render(scene.opaqueItems[i]);
	}

	m_renderer->popGPUPipelineState();
	m_renderer->popRenderTarget();
	m_renderer->popShadrProgram();
	m_passShader = nullptr;
	m_pass = RenderPass::None;
}



void DeferredRenderer::drawOpaquePass(const Scene_t& scene) {
	if (scene.numLights <= 0) {
		drawUnlitScene(scene);
	}
	else {
		for (size_t i = 0; i < scene.numLights; i++) {
			drawLightScene(scene, scene.lights[i]);
		}
	}

}

void DeferredRenderer::drawUnlitScene(const Scene_t& scene) {
	m_pass = RenderPass::UnlitPass;
	m_renderer->pushGPUPipelineState(&m_unlitPassPipelineState);

	auto unlitShader = ShaderProgramManager::getInstance()->getProgram("UnlitDeferred");
	if (unlitShader.expired())
		unlitShader = ShaderProgramManager::getInstance()->addProgram("UnlitDeferred.shader");
	ASSERT(!unlitShader.expired());

	m_passShader = unlitShader.lock();
	m_renderer->pushShaderProgram(m_passShader.get());

	m_diffuseBuffer->bind(Texture::Unit::DiffuseMap);
	m_emissiveBuffer->bind(Texture::Unit::EmissiveMap);
	m_passShader->setUniform1("u_diffuse", int(Texture::Unit::DiffuseMap));
	m_passShader->setUniform1("u_emissive", int(Texture::Unit::EmissiveMap));

	//draw a full screen quad
	m_renderer->drawFullScreenQuad();

	m_renderer->popGPUPipelineState();
	m_renderer->popShadrProgram();
	m_diffuseBuffer->unbind();
	m_emissiveBuffer->unbind();
	m_passShader = nullptr;
	m_pass = RenderPass::None;
}


void DeferredRenderer::drawLightScene(const Scene_t& scene, const Light_t& light) {
	if (light.isCastShadow()) {
		drawLightShadow(scene, light);
	}

	m_pass = RenderPass::LightPass;
	m_renderer->pushGPUPipelineState(&m_lightPassPipelineState);
	Buffer* lightUBO = nullptr;

	switch (light.type)
	{
	case LightType::DirectioanalLight: {
			auto directionalLightShader = ShaderProgramManager::getInstance()->getProgram("DirectionalLightDeferred");
			if (directionalLightShader.expired())
				directionalLightShader = ShaderProgramManager::getInstance()->addProgram("DirectionalLightDeferred.shader");
			ASSERT(!directionalLightShader.expired());

			m_passShader = directionalLightShader.lock();
			m_renderer->pushShaderProgram(m_passShader.get());
			
			// set directional light block
			if (m_passShader->hasUniformBlock("LightBlock")) {
				static DirectionalLightBlock dlb;
				dlb.color = glm::vec4(glm::vec3(light.color), light.intensity);
				dlb.inverseDiretion = -light.direction;
				m_directionalLightUBO->bind(Buffer::Target::UniformBuffer);
				m_directionalLightUBO->loadSubData(&dlb, 0, sizeof(dlb));

				m_directionalLightUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::LightBlock));
				m_passShader->bindUniformBlock("LightBlock", ShaderProgram::UniformBlockBindingPoint::LightBlock);

				lightUBO = m_directionalLightUBO.get();
			}

			if (m_passShader->hasUniform("u_VPMat")) {
				auto& camera = *scene.mainCamera;
				glm::mat4 vp = camera.projMatrix * camera.viewMatrix;
				m_passShader->setUniformMat4v("u_VPMat", &vp[0][0]);
			}
		} break;

	case LightType::PointLight: {
			auto pointLightShader = ShaderProgramManager::getInstance()->getProgram("PointLightDeferred");
			if (pointLightShader.expired())
				pointLightShader = ShaderProgramManager::getInstance()->addProgram("PointLightDeferred.shader");
			ASSERT(!pointLightShader.expired());

			m_passShader = pointLightShader.lock();
			m_renderer->pushShaderProgram(m_passShader.get());

			// set point light block
			if (m_passShader->hasUniformBlock("LightBlock")) {
				static PointLightBlock plb;
				plb.position = glm::vec4(light.position, light.range);
				plb.color = glm::vec4(light.color, light.intensity);

				m_pointLightUBO->bind(Buffer::Target::UniformBuffer);
				m_pointLightUBO->loadSubData(&plb, 0, sizeof(plb));
				m_pointLightUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::LightBlock));
				m_passShader->bindUniformBlock("LightBlock", ShaderProgram::UniformBlockBindingPoint::LightBlock);

				lightUBO = m_pointLightUBO.get();
			}
		}break;

	case LightType::SpotLight: {
			auto spotLightShader = ShaderProgramManager::getInstance()->getProgram("SpotLightDeferred");
			if (spotLightShader.expired())
				spotLightShader = ShaderProgramManager::getInstance()->addProgram("SpotLightDeferred.shader");
			ASSERT(!spotLightShader.expired());

			m_passShader = spotLightShader.lock();
			m_renderer->pushShaderProgram(m_passShader.get());

			// set spot light block
			if (m_passShader->hasUniformBlock("LightBlock")) {
				static SpotLightBlock slb;
				slb.position = glm::vec4(light.position, light.range);
				slb.color = glm::vec4(light.color, light.intensity);
				slb.inverseDirection = -light.direction;
				slb.angles = glm::vec2(light.innerCone, light.outterCone);

				m_spotLightUBO->bind(Buffer::Target::UniformBuffer);
				m_spotLightUBO->loadSubData(&slb, 0, sizeof(slb));
				m_spotLightUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::LightBlock));
				m_passShader->bindUniformBlock("LightBlock", ShaderProgram::UniformBlockBindingPoint::LightBlock);

				lightUBO = m_spotLightUBO.get();
			}
		}break;

	default:
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return;
	}

	m_shadowMappings[light.type]->beginLighttingPhase(light, m_passShader.get());

	// set camera position
	if (m_passShader->hasUniform("u_cameraPosW")) {
		glm::vec3 camPos = scene.mainCamera->position;
		m_passShader->setUniform3v("u_cameraPosW", &camPos[0]);
	}

	// set max shininess
	if (m_passShader->hasUniform("u_maxShininess")) {
		m_passShader->setUniform1("u_maxShininess", float(Material::s_maxShininess));
	}

	// set g-buffers
	if (m_passShader->hasUniform("u_posW")) {
		m_posWBuffer->bind(Texture::Unit::Position);
		m_passShader->setUniform1("u_posW", int(Texture::Unit::Position));
	}

	if (m_passShader->hasUniform("u_nromalW")) {
		m_normalWBuffer->bind(Texture::Unit::NormalMap);
		m_passShader->setUniform1("u_nromalW", int(Texture::Unit::NormalMap));
	}

	if (m_passShader->hasUniform("u_diffuse")) {
		m_diffuseBuffer->bind(Texture::Unit::DiffuseMap);
		m_passShader->setUniform1("u_diffuse", int(Texture::Unit::DiffuseMap));
	}

	if (m_passShader->hasUniform("u_specular")) {
		m_specularBuffer->bind(Texture::Unit::SpecularMap);
		m_passShader->setUniform1("u_specular", int(Texture::Unit::SpecularMap));
	}

	if (m_passShader->hasUniform("u_emissive")) {
		m_emissiveBuffer->bind(Texture::Unit::EmissiveMap);
		m_passShader->setUniform1("u_emissive", int(Texture::Unit::EmissiveMap));
	}

	m_renderer->drawFullScreenQuad();

	m_shadowMappings[light.type]->endLighttingPhase(light, m_passShader.get());

	if (lightUBO)
		lightUBO->unbind();

	m_posWBuffer->unbind();
	m_normalWBuffer->unbind();
	m_diffuseBuffer->unbind();
	m_specularBuffer->unbind();
	m_emissiveBuffer->unbind();

	m_renderer->popGPUPipelineState();
	m_renderer->popShadrProgram();
	m_passShader = nullptr;
	m_pass = RenderPass::None;
}


void DeferredRenderer::drawLightShadow(const Scene_t& scene, const Light_t& light) {
	m_pass = RenderPass::ShadowPass;
	m_renderer->pushGPUPipelineState(&m_shadowPassPipelineState);
	m_renderer->setColorMask(false);

	if (m_shadowMappings.find(light.type) == m_shadowMappings.end()) {
		IShadowMapping* shadowMapping = nullptr;
		if (light.type == LightType::DirectioanalLight) {
			shadowMapping = new DirectionalLightShadowMapping(m_renderer->getRenderTechnique(), m_renderer->getShadowMapResolution());
		}
		else if (light.type == LightType::PointLight) {
			shadowMapping = new PointLightShadowMapping(m_renderer->getRenderTechnique(), m_renderer->getShadowMapResolution());
		}
		else if (light.type == LightType::SpotLight) {
			shadowMapping = new SpotLightShadowMapping(m_renderer->getRenderTechnique(), m_renderer->getShadowMapResolution());
		}
		else {
			ASSERT(false);
		}
		ASSERT(shadowMapping->initialize());

		m_shadowMappings.insert(std::make_pair(light.type, std::unique_ptr<IShadowMapping>(shadowMapping)));
	}

	m_shadowMappings[light.type]->beginShadowPhase(scene, light);
	m_shadowMappings[light.type]->endShadowPhase();

	m_renderer->popGPUPipelineState();
	m_renderer->setColorMask(true);
	m_passShader = nullptr;
	m_pass = RenderPass::None;
}


void DeferredRenderer::render(const MeshRenderItem_t& task) {
	auto taskExecutor = m_taskExecutors.find(m_pass);
	if (taskExecutor == m_taskExecutors.end()) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		std::stringstream msg;
		msg << "renderer no task executor for pass: " << int(m_pass) << "\n";
		CONSOLELOG(msg.str());
		return;
	}

	taskExecutor->second->executeMeshTask(task, m_renderer->getActiveShaderProgram());
}


void DeferredRenderer::onWindowResize(float w, float h) {
	if (w <= 0 || h <= 0)
		return;
	setupGBuffers();
}


void DeferredRenderer::onShadowMapResolutionChange(float w, float h) {
	for (auto& shadowMapping : m_shadowMappings) {
		shadowMapping.second->onShadowMapResolutionChange(w, h);
	}
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
	glm::vec2 renderSize = m_renderer->getRenderSize();
	success = m_posWBuffer->loadImage2DFromMemory(Texture::Format::RGBA16F, 
													Texture::Format::RGBA,
													Texture::FormatDataType::Float, 
													renderSize.x,
													renderSize.y,
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
														renderSize.x,
														renderSize.y,
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
														renderSize.x,
														renderSize.y,
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
														renderSize.x,
														renderSize.y,
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
														renderSize.x,
														renderSize.y,
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
															renderSize.x,
															renderSize.y,
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

