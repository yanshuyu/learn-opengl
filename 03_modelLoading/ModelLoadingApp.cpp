#include"ModelLoadingApp.h"


ModelLoadingApp::ModelLoadingApp(const std::string& t, int w, int h) :GLApplication(t, w, h)
, m_scene(nullptr)
, m_renderer(nullptr) {

}

bool ModelLoadingApp::initailize() {
	if (!__super::initailize())
		return false;

	auto shaderMgr = ShaderProgramManager::getInstance();
	auto meshMgr = MeshManager::getInstance();
	auto matMgr = MaterialManager::getInstance();
	auto texMgr = TextureManager::getInstance();

	auto unlitShader = shaderMgr->addProgram(shaderMgr->getResourceAbsolutePath() + "Unlit.shader");
	auto monsterModel = meshMgr->addModel(meshMgr->getResourceAbsolutePath() + "Alien_Animal.fbx", MeshLoadOption::LoadMaterial);
	auto diffuseTexture = texMgr->addTexture(texMgr->getResourceAbsolutePath() + "REF 1.jpg");

	m_scene = std::make_unique<Scene>(glm::vec2(m_wndWidth, m_wndHeight), "model_loading_demo_scene");
	m_renderer = std::unique_ptr<Renderer>(new Renderer(m_wndWidth, m_wndHeight, Renderer::Mode::Forward));
	
	//m_renderer->setClearColor(0, 0, 1, 1);

	auto obj = m_scene->addObject("monster");
	auto meshRender = MeshRenderComponent::create();
	meshRender->setMeshes(monsterModel);
	if (!obj->addComponent(meshRender)) {
		MeshRenderComponent::destory(meshRender);
		ASSERT(false);
	}
	obj->m_transform.setScale({ 0.01f, 0.01f, 0.01f });
	
	auto camera = m_scene->addCamera(glm::vec3(0, 0, 8));
	camera->addComponent(FirstPersonCameraController::create());

	ASSERT(m_scene->initialize());
	//ASSERT(m_renderer->initialize());
}


void ModelLoadingApp::update(double dt) {
	m_scene->update(dt);
}


void ModelLoadingApp::render() {
	m_renderer->renderScene(m_scene.get());
}

void ModelLoadingApp::onWindowResized(int width, int height) {
	__super::onWindowResized(width, height);
	m_scene->onWindowReSize(width, height);
}
