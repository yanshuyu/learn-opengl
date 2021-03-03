#include"RenderTechnique.h"
#include"Renderer.h"
#include"ShaderProgamMgr.h"
#include"VertexArray.h"



RenderTechniqueBase::RenderTechniqueBase(Renderer* renderer) :IRenderTechnique(renderer)
, m_pass(RenderPass::None)
, m_passShader()
, m_skyBoxPipelineState() {
	m_skyBoxPipelineState.cullMode = CullFaceMode::None;
	m_skyBoxPipelineState.depthMode = DepthMode::Enable;
	m_skyBoxPipelineState.depthFunc = DepthFunc::LEqual;
	m_skyBoxPipelineState.depthMask = 0;
}


void RenderTechniqueBase::render(const Scene_t& scene) {
	beginFrame();

	drawDepthPass(scene);
	drawGeometryPass(scene);
	drawOpaquePass(scene);
	if (scene.skyBox) drawSkyBox(*scene.skyBox, *scene.mainCamera);
	drawTransparentPass(scene);

	endFrame();
}


void RenderTechniqueBase::render(const SkyBox_t& skyBox) {
	m_renderer->pushGPUPipelineState(&m_skyBoxPipelineState);

	skyBox.cubeMap->bindToTextureUnit(Texture::Unit::CubeMap, Texture::Target::Texture_CubeMap);
	m_renderer->getActiveShaderProgram()->setUniform1("u_CubeMap", int(Texture::Unit::CubeMap));
	skyBox.cubeVAO->bind();
	GLCALL(glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0));
	skyBox.cubeVAO->unbind();
	skyBox.cubeMap->unbindFromTextureUnit();

	m_renderer->popGPUPipelineState();
}


void RenderTechniqueBase::drawSkyBox(const SkyBox_t& skyBox, const Camera_t& camera) {
	auto shader_weak = ShaderProgramManager::getInstance()->getProgram("SkyBox");
	if (shader_weak.expired())
		shader_weak = ShaderProgramManager::getInstance()->addProgram("SkyBox");

	ASSERT(!shader_weak.expired());
	auto shader = shader_weak.lock();

	m_renderer->pushShaderProgram(shader.get());
	glm::mat4 vp = camera.projMatrix * glm::mat4(glm::mat3(camera.viewMatrix));
	shader->setUniformMat4v("u_VP", &vp[0][0]);
	
	render(skyBox);
	m_renderer->popShadrProgram();
}