#include"ImageProcessingApp.h"
#include"MainGuiWindow.h"
#include<common/FileSystem.h>
#include<stdarg.h>
#include<glm/gtx/transform.hpp>



ImageProcessingApp::ImageProcessingApp(const std::string& t, int w, int h) :GLApplication(t, w, h)
, m_scene(nullptr)
, m_renderer(nullptr)
, m_animator() {

}

bool ImageProcessingApp::initailize() {
	if (!__super::initailize())
		return false;

	m_scene = std::unique_ptr<Scene>(new Scene("image_processing_secene"));
	m_renderer = std::unique_ptr<Renderer>(new Renderer(glm::vec2(m_wndWidth, m_wndHeight)));
	m_renderer->setRenderMode(Renderer::Mode::Forward);
	ASSERT(m_renderer->isValid());
	m_scene->setRenderer(m_renderer.get());

	auto shaderMgr = ShaderProgramManager::getInstance();
	auto meshMgr = MeshManager::getInstance();
	auto matMgr = MaterialManager::getInstance();
	auto texMgr = TextureManager::getInstance();

	// cube
	auto cubeTexture = texMgr->addTexture("wall.jpg");
	auto cubeMat = matMgr->addMaterial("CubeMaterial").lock();
	cubeMat->m_diffuseMap = cubeTexture;
	cubeMat->m_shininess = 0.1f;
	auto cube = m_scene->addCube(cubeMat);
	cube->m_transform.setScale({ 4.f, 4.f, 4.f });
	cube->m_transform.setPosition({ -40.f, 0.f, 10.f });

	// plane
	auto planeMat = matMgr->addMaterial("PlaneMaterial").lock();
	planeMat->m_shininess = 0.1f;
	m_scene->addPlane(250, 250, planeMat);

	// model
	auto obj = m_scene->addModel("Alien_Animal.fbx");
	obj->m_transform.setScale({ 0.01f, 0.01f, 0.01f });
	obj->m_transform.setPosition({ 0.f, 0.f, 0.f });
	obj->setTag(100);
	obj->addComponent<AnimatorController>();
	m_animator = obj->getComponent<AnimatorComponent>();

	// camera
	auto camera = m_scene->addCamera({ 0.f, 4.f, 16.f });
	camera->addComponent<FirstPersonCameraController>();
	camera->addComponent<HDRFilterComponent>();

	//camera->addComponent(ArcballCameraController::create());
	//auto cameraController = camera->getComponent<ArcballCameraController>();
	//cameraController.lock()->setPosition({ 0.f, 4.f, 16.f });

	// light
	auto dirLight = m_scene->addDirectionalLight({ 0.9f, 0.9f, 0.9f }, 0.9f, ShadowType::SoftShadow);
	dirLight->m_transform.setRotation({ -30.f , -60.f, 0.f });

	//auto pointLight = m_scene->addPointLight({ 1.f, 1.f, 0.8f }, 80.f, 1.f, ShadowType::SoftShadow);
	//pointLight->m_transform.setPosition({-20.f, 40.f, -10.f});

	//auto spotLight = m_scene->addSpotLight({ 1.f, 1.f, 0.8f }, 45.f, 65.f, 80.f, 1.f, ShadowType::SoftShadow);
	//spotLight->m_transform.setPosition({ -15.f, 40.f, 10.f });
	//spotLight->m_transform.setRotation({ -45.f, 0.f, 0.f });

	GuiManager::getInstance()->addWindow(new MainGuiWindow(m_scene->getName(), this));


	return true;
}


void ImageProcessingApp::update(double dt) {
	if (!m_scene->isInitialize()) {
		ASSERT(m_renderer->initialize());
		ASSERT(m_scene->initialize());
		ASSERT(GuiManager::getInstance()->initialize());
	}

	m_scene->update(dt);
}


void ImageProcessingApp::render() {
	m_scene->render();
}



void ImageProcessingApp::onWindowResized(int width, int height) {
	__super::onWindowResized(width, height);
	m_scene->onWindowResize(width, height);
	m_renderer->onWindowResize(width, height);
}
