#include"ForwardRenderer.h"
#include"ShaderProgamMgr.h"
#include"CameraComponent.h"
#include"Scene.h"


ForwardRenderer::ForwardRenderer() :Renderer()
, m_renderContext(this)
, m_renderingScene(nullptr)
, m_unlitShader(nullptr)
, m_camera() {

}


bool ForwardRenderer::initialize() {
	GLCALL(glEnable(GL_DEPTH_TEST));
    GLCALL(glDepthFunc(GL_LESS));

	GLCALL(glEnable(GL_CULL_FACE));
	GLCALL(glCullFace(GL_BACK));
	GLCALL(glFrontFace(GL_CCW));

	return true;
}


void ForwardRenderer::renderScene(Scene* s) {
	m_renderingScene = s;
	s->preRender(this);

	m_camera = s->getCamera()->makeCamera();
	setClearColor(m_camera.backgrounColor.r, m_camera.backgrounColor.g, m_camera.backgrounColor.b, m_camera.backgrounColor.a);
	setViewPort(m_camera.viewPortX, m_camera.viewportY, m_camera.viewportWidth, m_camera.viewportHeight);
	clearScrren();
	m_renderContext.clearMatrix();
	
	beginUnlitPass();
	s->render(&m_renderContext);
	endUnlitPass();
	
	s->afterRender(this);
}

void ForwardRenderer::subsimtTask(const RenderTask_t& task) {
	prepareTask(task);
	performTask(task);
}

void ForwardRenderer::beginUnlitPass() {
	if (!m_unlitShader) {
		m_unlitShader = ShaderProgramManager::getInstance()->getProgram("Unlit");
		if (!m_unlitShader)
			m_unlitShader = ShaderProgramManager::getInstance()->addProgram("Unlit", "C:/Users/SY/Documents/learn-opengl/res/shader/Unlit.shader");
	}
	ASSERT(m_unlitShader != nullptr);
	m_activeShader = m_unlitShader;
	m_unlitShader->bind();
}


void ForwardRenderer::endUnlitPass() {
	//m_unlitShader->unbind();
}

void ForwardRenderer::beginLightpass(const Light_t& light) {

}

void ForwardRenderer::endLightPass() {

}

void ForwardRenderer::prepareTask(const RenderTask_t& task) {
	task.vao->bind();
	
	// set mvp matrix
	if (m_activeShader->hasUniform("u_mvp")) {
		glm::mat4 mvp = m_camera.projMatrix * m_camera.viewMatrix * task.modelMatrix;
		m_activeShader->setUniformMat4v("u_mvp", &mvp[0][0]);
	}
	
	// set material
	if (m_activeShader->hasUniform("u_diffuseColor")) {
		m_activeShader->setUniform4("u_diffuseColor",
			task.material->m_diffuseColor.r,
			task.material->m_diffuseColor.g,
			task.material->m_diffuseColor.b,
			task.material->m_opacity);
	}

	// set textures
	if (task.material->hasDiffuseTexture()) {
		task.material->m_diffuseMap->bind(0);
		m_activeShader->setUniform1("u_diffuseMap", 0);
	}
}

void ForwardRenderer::performTask(const RenderTask_t& task) {
	switch (task.primitive)
	{
	case PrimitiveType::Triangle: {
		if (task.indexCount > 0) {
			GLCALL(glDrawElements(GL_TRIANGLES, task.indexCount, GL_UNSIGNED_INT, 0));
		} else {
			GLCALL(glDrawArrays(GL_TRIANGLES, 0, task.vertexCount / 3));
		}
	} break;

	default:
		break;
	}
}