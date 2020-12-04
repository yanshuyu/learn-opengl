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
, m_geometryBufferTarget(renderer->getRenderSize())
, m_frameTarget(renderer->getRenderSize())
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
	ok = setupRenderTargets();

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
	

	return ok;
}


void DeferredRenderer::cleanUp() {
	m_directionalLightUBO.release();
	m_pointLightUBO.release();
	m_spotLightUBO.release();

	m_shadowMappings.clear();

	m_geometryBufferTarget.detachAllTexture();
	m_frameTarget.detachAllTexture();
}


void DeferredRenderer::beginFrame() {
	m_renderer->pushRenderTarget(&m_geometryBufferTarget);
	m_renderer->clearScreen(ClearFlags::Color | ClearFlags::Depth | ClearFlags::Stencil);
}

void DeferredRenderer::endFrame() {
	m_renderer->popRenderTarget(); 

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
	
	//m_renderer->popRenderTarget();
	m_renderer->popGPUPipelineState();
	m_renderer->popShadrProgram();
	m_passShader = nullptr;
	m_pass = RenderPass::None;
}



void DeferredRenderer::drawOpaquePass(const Scene_t& scene) {
	m_renderer->pushRenderTarget(&m_frameTarget);
	m_renderer->clearScreen(ClearFlags::Color);

	if (scene.numLights <= 0) {
		drawUnlitScene(scene);
	}
	else {
		for (size_t i = 0; i < scene.numLights; i++) {
			drawLightScene(scene, scene.lights[i]);
		}
	}

	m_renderer->popRenderTarget();
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

	Texture* diffuse = m_geometryBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 2);
	Texture* emissive = m_geometryBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 4);
	
#ifdef _DEBUG
	ASSERT(diffuse && emissive);
#endif // _DEBUG

	if (diffuse) {
		diffuse->bind(Texture::Unit::DiffuseMap);
		m_passShader->setUniform1("u_diffuse", int(Texture::Unit::DiffuseMap));
	}

	if (emissive) {
		emissive->bind(Texture::Unit::EmissiveMap);
		m_passShader->setUniform1("u_emissive", int(Texture::Unit::EmissiveMap));
	}
	//draw a full screen quad
	m_renderer->drawFullScreenQuad();

	m_renderer->popGPUPipelineState();
	m_renderer->popShadrProgram();
	if (diffuse) diffuse->unbind();
	if (emissive) emissive->unbind();
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
	Texture* pos = m_geometryBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 0);
	Texture* normal = m_geometryBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 1);
	Texture* diffuse = m_geometryBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 2);
	Texture* specular = m_geometryBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 3);
	Texture* emissive = m_geometryBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 4);

#ifdef _DEBUG
	ASSERT(pos&& normal&& diffuse&& specular&& emissive);
#endif // _DEBUG


	if (m_passShader->hasUniform("u_posW") && pos) {		
		pos->bind(Texture::Unit::Position);
		m_passShader->setUniform1("u_posW", int(Texture::Unit::Position));
	}

	if (m_passShader->hasUniform("u_nromalW") && normal) {
		normal->bind(Texture::Unit::NormalMap);
		m_passShader->setUniform1("u_nromalW", int(Texture::Unit::NormalMap));
	}

	if (m_passShader->hasUniform("u_diffuse") && diffuse) {
		diffuse->bind(Texture::Unit::DiffuseMap);
		m_passShader->setUniform1("u_diffuse", int(Texture::Unit::DiffuseMap));
	}

	if (m_passShader->hasUniform("u_specular") && specular) {
		specular->bind(Texture::Unit::SpecularMap);
		m_passShader->setUniform1("u_specular", int(Texture::Unit::SpecularMap));
	}

	if (m_passShader->hasUniform("u_emissive") && emissive) {
		emissive->bind(Texture::Unit::EmissiveMap);
		m_passShader->setUniform1("u_emissive", int(Texture::Unit::EmissiveMap));
	}

	m_renderer->drawFullScreenQuad();

	m_shadowMappings[light.type]->endLighttingPhase(light, m_passShader.get());

	if (lightUBO)
		lightUBO->unbind();

	if (pos) pos->unbind();
	if (normal) normal->unbind();
	if (diffuse) diffuse->unbind();
	if (specular) specular->unbind();
	if (emissive) emissive->unbind();

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

	m_geometryBufferTarget.resize({ w, h });
	m_frameTarget.resize({ w, h });
}


void DeferredRenderer::onShadowMapResolutionChange(float w, float h) {
	for (auto& shadowMapping : m_shadowMappings) {
		shadowMapping.second->onShadowMapResolutionChange(w, h);
	}
}


bool DeferredRenderer::setupRenderTargets() {
	// world position
	if (!m_geometryBufferTarget.attachTexture2D(Texture::Format::RGBA16F, Texture::Format::RGBA, Texture::FormatDataType::Float, RenderTarget::Slot::Color, 0)) {
		cleanUp();
#ifdef _DEBUG
		ASSERT(false);
#endif 
		return false;
	}
	
	Texture* buffer = m_geometryBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 0);
#ifdef _DEBUG
	ASSERT(buffer);
#endif 
	if (buffer) {
		buffer->bind();
		buffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
		buffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
		buffer->unbind();
	}


	// world normal
	if (!m_geometryBufferTarget.attachTexture2D(Texture::Format::RGBA16F, Texture::Format::RGBA, Texture::FormatDataType::Float, RenderTarget::Slot::Color, 1)) {
		cleanUp();
#ifdef _DEBUG
		ASSERT(false);
#endif 
		return false;
	}
	
	buffer = m_geometryBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 1);
#ifdef _DEBUG
	ASSERT(buffer);
#endif 
	if (buffer) {
		buffer->bind();
		buffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
		buffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
		buffer->unbind();
	}

	// diffuse color
	if (!m_geometryBufferTarget.attachTexture2D(Texture::Format::RGBA, Texture::Format::RGBA, Texture::FormatDataType::UByte, RenderTarget::Slot::Color, 2)) {
		cleanUp();
#ifdef _DEBUG
		ASSERT(false);
#endif 
		return false;
	}

	buffer = m_geometryBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 2);
#ifdef _DEBUG
	ASSERT(buffer);
#endif 
	if (buffer) {
		buffer->bind();
		buffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
		buffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
		buffer->unbind();
	}


	// specular color
	if (!m_geometryBufferTarget.attachTexture2D(Texture::Format::RGBA, Texture::Format::RGBA, Texture::FormatDataType::UByte, RenderTarget::Slot::Color, 3)) {
		cleanUp();
#ifdef _DEBUG
		ASSERT(false);
#endif 
		return false;
	}

	buffer = m_geometryBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 3);
#ifdef _DEBUG
	ASSERT(buffer);
#endif 

	if (buffer) {
		buffer->bind();
		buffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
		buffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
		buffer->unbind();
	}

	// emissive color
	if (!m_geometryBufferTarget.attachTexture2D(Texture::Format::RGBA, Texture::Format::RGBA, Texture::FormatDataType::UByte, RenderTarget::Slot::Color, 4)) {
		cleanUp();
#ifdef _DEBUG
		ASSERT(false);
#endif 
		return false;
	}

	buffer = m_geometryBufferTarget.getAttachedTexture(RenderTarget::Slot::Color, 4);
#ifdef _DEBUG
	ASSERT(buffer);
#endif 
	
	if (buffer) {
		buffer->bind();
		buffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
		buffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
		buffer->unbind();
	}

	// depth/stcencil
	if (!m_geometryBufferTarget.attachTexture2D(Texture::Format::Depth24_Stencil8, Texture::Format::Depth_Stencil, Texture::FormatDataType::UInt_24_8, RenderTarget::Slot::Depth_Stencil)) {
		cleanUp();
#ifdef _DEBUG
		ASSERT(false);
#endif 
		return false;
	}

	buffer = m_geometryBufferTarget.getAttachedTexture(RenderTarget::Slot::Depth_Stencil);
#ifdef _DEBUG
	ASSERT(buffer);
#endif 
	if (buffer) {
		buffer->bind();
		buffer->setFilterMode(Texture::FilterType::Minification, Texture::FilterMode::Nearest);
		buffer->setFilterMode(Texture::FilterType::Magnification, Texture::FilterMode::Nearest);
		buffer->unbind();
	}

	m_geometryBufferTarget.setDrawLocations({ 0, 1, 2, 3, 4 });

	if (!m_geometryBufferTarget.isValid()) {
#ifdef _DEBUG
		ASSERT(false);
#endif
		return false;
	}


	if (!m_frameTarget.attachTexture2D(Texture::Format::RGBA16F, Texture::Format::RGBA, Texture::FormatDataType::Float, RenderTarget::Slot::Color)) {
		cleanUp();
#ifdef _DEBUG
		ASSERT(false);
#endif
		return false;
	}



	return true;
}

