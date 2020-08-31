#include"TextureCubeApp.h"
#include<common/pch.h>


TextureCubeApp::TextureCubeApp(const std::string& title) :GLApplication(title)
, m_vbo(nullptr)
, m_ibo(nullptr)
, m_vao(nullptr)
, m_shaderProgram(nullptr)
, m_abedoMap(nullptr) {
}


bool TextureCubeApp::initailize() {
	bool success = __super::initailize();

	if (success) {



		float vertices[] = { -0.5, 0.4, 0.0, 1.0,
							0.5, 0.4, 1.0, 1.0,
							0.5, -0.4, 1.0, 0.0,
							-0.5, -0.4, 0.0, 0.0 };

		unsigned int indices[] = { 0, 1, 2,
									0, 2, 3 };

		//GLCALL(glGenVertexArrays(1, &m_vao));
		//glBindVertexArray(m_vao);

		m_vao = std::make_shared<VertexArray>();
		m_vbo = std::make_shared<Buffer>(vertices, sizeof(float) * 16, GL_ARRAY_BUFFER, GL_STATIC_DRAW);
		
		VertexLayoutDescription vertexLayoutDesc;
		vertexLayoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 2);
		vertexLayoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 2);

		m_vao->storeVertexLayout(*m_vbo, vertexLayoutDesc);

		//m_vbo->bind();

		//GLCALL(glGenBuffers(1, &m_vbo));
		//GLCALL(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
		//GLCALL(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, vertices, GL_STATIC_DRAW));

		//int offset = 0;
		//GLCALL(glEnableVertexAttribArray(0));
		//GLCALL(glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 4, reinterpret_cast<void*>(offset)));

		//offset = sizeof(float) * 2;
		//GLCALL(glEnableVertexAttribArray(1));
		//GLCALL(glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 4, reinterpret_cast<void*>(offset)));

		//GLCALL(glBindVertexArray(0));

		//GLCALL(glGenBuffers(1, &m_ibo));
		//GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo));
		//GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, indices, GL_STATIC_DRAW));

		m_ibo = std::make_shared<Buffer>(indices, sizeof(float) * 6, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
		
		auto programMgr = ShaderProgramManager::getInstance();
		ASSERT(programMgr->addProgram("SolidColor", "res/shader/SolidColor.shader"));
		ASSERT(programMgr->addProgram("AbedoMap", "res/shader/AbedoMap.shader"));
		//ASSERT(programMgr->addProgram("test", "res/shader/test.shader"));
		
		m_shaderProgram = programMgr->getProgram("AbedoMap");
		

		auto textureMgr = TextureManager::getInstance();
		ASSERT(textureMgr->addTexture("opengl_logo", "res/images/opengl_logo.png"));
		m_abedoMap = textureMgr->getTexture("opengl_logo");

		GLCALL(glEnable(GL_CULL_FACE));
		GLCALL(glCullFace(GL_BACK));
		GLCALL(glFrontFace(GL_CW));

		GLCALL(glEnable(GL_BLEND));
		GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		GLCALL(glBlendEquation(GL_FUNC_ADD));
	}
	
	return success;
}


void TextureCubeApp::update(double dt) {

}



void TextureCubeApp::render() {
	GLCALL(glClearColor(0.0, 0.0, 0.0, 1.0));
	GLCALL(glClearDepth(1.0));
	GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));


	//GLCALL(glBindVertexArray(m_vao));
	//GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo));
	m_vao->bind();
	m_ibo->bind();
	m_shaderProgram->bind();
	m_abedoMap->bind(0);

	static float color[] = { 1.0, 0.0, 0.0, 1.0 };
	//GLCALL(m_shaderProgram->setUniform4v("u_mainColor", color));
	GLCALL(m_shaderProgram->setUniform1("u_abedoMap", 0));

	GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
}
	