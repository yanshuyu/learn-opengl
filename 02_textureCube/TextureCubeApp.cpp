#include"TextureCubeApp.h"
#include<glm/ext/matrix_transform.hpp>
#include<glm/ext/matrix_clip_space.hpp>



TextureCubeApp::TextureCubeApp(const std::string& title) :GLApplication(title)
, m_vbo_cube(nullptr)
, m_ibo_cube(nullptr)
, m_vao_cube(nullptr)
, m_vbo_qaud(nullptr)
, m_ibo_qaud(nullptr)
, m_vao_qaud(nullptr)
, m_shaderProgram(nullptr)
, m_abedoMap(nullptr)
, m_modelMat(1)
, m_viewMat(1)
, m_projMat(1)
, m_orthMat(1)
, m_camera(nullptr) {
}


bool TextureCubeApp::initailize() {
	bool success = __super::initailize();

	if (success) {
		Vertex cubeVertices[] = {
			// front
			Vertex({-1, -1, 1}, {0.6, 0, 0}, {0, 0}),
			Vertex({1, -1, 1}, {0, 0.6, 0}, {1, 0}),
			Vertex({1, 1, 1}, {0, 0, 0.6}, {1, 1}),
			Vertex({-1, 1, 1}, {0.0, 0.0, 0.0}, {0, 1}),

			//top
			Vertex({-1, 1, 1}, {0.0, 0.0, 0}, {0, 0}),
			Vertex({1, 1, 1}, {0, 0, 0.6}, {1, 0}), 
			Vertex({1, 1, -1}, {0.6, 0.6, 0}, {1, 1}), 
			Vertex({-1, 1, -1}, {0.0, 0.6, 0.6}, {0, 1}), 

			// back
			Vertex({1, -1, -1}, {0.0, 0.0, 0}, {0, 0}),
			Vertex({-1, -1, -1}, {0.6, 0, 0.6}, {1, 0}), 
			Vertex({-1, 1, -1}, {0.0, 0.6, 0.6}, {1, 1}),
			Vertex({1, 1, -1}, {0.6, 0.6, 0}, {0, 1}),

			//bottom
			Vertex({-1, -1, -1}, {0.6, 0, 0.6}, {0, 0}),
			Vertex({1, -1, -1}, {0.0, 0.0, 0}, {1, 0}),
			Vertex({1, -1, 1}, {0, 0.6, 0}, {1, 1}),
			Vertex({-1, -1, 1}, {0.0, 0.0, 0.0}, {0, 1}),

			// left
			Vertex({-1, -1, -1}, {0.6, 0, 0.6}, {0, 0}),
			Vertex({-1, -1, 1}, {0.6, 0, 0}, {1, 0}),
			Vertex({-1, 1, 1}, {0.0, 0.0, 0}, {1, 1}),
			Vertex({-1, 1, -1}, {0.0, 0.6, 0.6}, {0, 1}),

			//right
			Vertex({1, -1, 1}, {0, 0.6, 0}, {0, 0}),
			Vertex({1, -1, -1}, {0.0, 0.0, 0}, {1, 0}),
			Vertex({1, 1, -1}, {0.6, 0.6, 0}, {1, 1}),
			Vertex({1, 1, 1}, {0, 0, 0.6}, {0, 1}),
		};

		unsigned int cubeIndices[] = {
			// front
			0,  1,  2,
			2,  3,  0,
			// top
			4,  5,  6,
			6,  7,  4,
			// back
			8,  9, 10,
			10, 11,  8,
			// bottom
			12, 13, 14,
			14, 15, 12,
			// left
			16, 17, 18,
			18, 19, 16,
			// right
			20, 21, 22,
			22, 23, 20,
		};

		Vertex quadVertices[] = {
			Vertex({0, 0, 0}, {0, 0, 0}, {0, 0}),
			Vertex({250, 0, 0}, {0, 0, 0}, {1, 0}),
			Vertex({250, 120, 0}, {0, 0, 0}, {1, 1}),
			Vertex({0, 120, 0}, {0, 0, 0}, {0, 1}),
		};

		unsigned int quadIndices[] = {
			0,1,2,
			2,3,0
		};

		m_vao_cube = std::make_unique<VertexArray>();
		m_vbo_cube = std::make_unique<Buffer>(cubeVertices, sizeof(Vertex) * 24, GL_ARRAY_BUFFER, GL_STATIC_DRAW);
		m_ibo_cube = std::make_unique<Buffer>(cubeIndices, sizeof(unsigned int) * 36, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, 36);

		VertexLayoutDescription vertexLayoutDesc;
		vertexLayoutDesc.setStride(sizeof(Vertex));
		vertexLayoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 3);
		vertexLayoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 3);
		vertexLayoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 2);
		
		m_vao_cube->bind();
		m_vbo_cube->bind();
		m_ibo_cube->bind();
		m_vao_cube->storeVertexLayout(vertexLayoutDesc);
		m_vao_cube->unbind();
		m_ibo_cube->unbind();
		m_vbo_cube->unbind();

		m_vao_qaud = std::make_unique<VertexArray>();
		m_vbo_qaud = std::make_unique<Buffer>(quadVertices, sizeof(Vertex) * 4, GL_ARRAY_BUFFER, GL_STATIC_DRAW);
		m_ibo_qaud = std::make_unique<Buffer>(quadIndices, sizeof(unsigned int) * 6, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, 6);

		m_vao_qaud->bind();
		m_vbo_qaud->bind();
		m_ibo_qaud->bind();
		m_vao_qaud->storeVertexLayout(vertexLayoutDesc);
		m_vao_qaud->unbind();
		m_ibo_qaud->unbind();
		m_vao_qaud->unbind();


		auto programMgr = ShaderProgramManager::getInstance();
		ASSERT(programMgr->addProgram("res/shader/SolidColor.shader"));
		ASSERT(programMgr->addProgram("res/shader/AbedoMap.shader"));
		//ASSERT(programMgr->addProgram("test", "res/shader/test.shader"));
		m_shaderProgram = programMgr->getProgram("AbedoMap");
		

		auto textureMgr = TextureManager::getInstance();
		ASSERT(textureMgr->addTexture("res/images/opengl_logo.png"));
		m_abedoMap = textureMgr->getTexture("opengl_logo.png");

		m_modelMat = glm::rotate(m_modelMat, glm::radians(30.f), glm::vec3(0, 1, 0));
		m_modelMat = glm::rotate(m_modelMat, glm::radians(15.f), glm::vec3(1, 0, 0));
		m_viewMat = glm::lookAt(glm::vec3(0,0,8), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
		m_projMat = glm::perspective(glm::radians(45.f), float(m_wndWidth) /m_wndHeight, 0.1f, 100.0f);

		m_orthMat = glm::ortho(0.f, float(m_wndWidth), 0.f, float(m_wndHeight), float(-1), 1.f);

		m_camera = std::make_unique<SceneObject>("camera");
		m_camera->addComponent(new CameraComponent(m_wndWidth, m_wndHeight));
		m_camera->addComponent(FirstPersonCameraController::create());
		m_camera->m_transform.setPosition(glm::vec3(0, 0, 8));

		GLCALL(glEnable(GL_CULL_FACE));
		GLCALL(glCullFace(GL_BACK));
		GLCALL(glFrontFace(GL_CCW));

		GLCALL(glEnable(GL_BLEND));
		GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		GLCALL(glBlendEquation(GL_FUNC_ADD));
	}
	
	return success;
}


void TextureCubeApp::onWindowResized(int w, int h) {
	__super::onWindowResized(w, h);
	GLCALL(glViewport(0, 0, m_wndWidth, m_wndHeight));
	//m_projMat = glm::perspective(glm::radians(45.f), float(m_wndWidth) / m_wndHeight, 0.1f, 100.0f);
	//m_orthMat = glm::ortho(0.f, float(m_wndWidth), 0.f, float(m_wndHeight), float(-1), 1.f);
}


void TextureCubeApp::update(double dt) {
	float ry = glm::radians(15.f * dt);
	m_modelMat = glm::rotate(m_modelMat, ry, glm::vec3(1, 0, 0));
	m_modelMat = glm::rotate(m_modelMat, ry, glm::vec3(0, 1, 0));
	m_camera->update(dt);
}



void TextureCubeApp::render() {
	GLCALL(glClearColor(0.0, 0.0, 0.0, 1.0));
	GLCALL(glClearDepth(1.0));
	GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	GLCALL(glEnable(GL_DEPTH_TEST));
	m_vao_cube->bind();
	m_shaderProgram->bind();
	m_abedoMap->bind(0);

	auto cameraComponent = static_cast<CameraComponent*>(m_camera->findComponent(CameraComponent::s_indentifier));
	glm::mat4 mvp = cameraComponent->viewProjectionMatrix() * m_modelMat;
	GLCALL(m_shaderProgram->setUniformMat4v("MVP", &mvp[0][0]));
	GLCALL(m_shaderProgram->setUniform1("u_abedoMap", 0));
	GLCALL(glDrawElements(GL_TRIANGLES, m_ibo_cube->getElementCount(), GL_UNSIGNED_INT, 0));
	

	GLCALL(glDisable(GL_DEPTH_TEST));
	m_vao_qaud->bind();
	
	mvp = glm::mat4(1);
	mvp = glm::translate(mvp, glm::vec3(20, 20, 0));
	mvp = m_orthMat * mvp;
	GLCALL(m_shaderProgram->setUniformMat4v("MVP", &mvp[0][0]));
	GLCALL(glDrawElements(GL_TRIANGLES, m_ibo_qaud->getElementCount(), GL_UNSIGNED_INT, 0));

}
	