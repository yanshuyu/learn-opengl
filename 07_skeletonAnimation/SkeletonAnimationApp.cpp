#include"SkeletonAnimationApp.h"
#include"LightControlGuiWindow.h"


SkeletonAnimationApp::SkeletonAnimationApp(const std::string& t, int w, int h) :GLApplication(t, w, h)
, m_scene(nullptr)
, m_renderer(nullptr) {

}

bool SkeletonAnimationApp::initailize() {
	if (!__super::initailize())
		return false;

	RenderingSettings_t renderSettins;
	renderSettins.renderSize = { m_wndWidth, m_wndHeight };
	renderSettins.shadowMapResolution = { 1024.f, 1024.f };
	m_scene = std::make_unique<Scene>(glm::vec2(m_wndWidth, m_wndHeight), "deferred_rendering_scene");
	m_renderer = std::unique_ptr<Renderer>(new Renderer(renderSettins));

	m_renderer->setRenderMode(Renderer::Mode::Forward);
	ASSERT(m_renderer->isValid());

	auto shaderMgr = ShaderProgramManager::getInstance();
	auto meshMgr = MeshManager::getInstance();
	auto matMgr = MaterialManager::getInstance();
	auto texMgr = TextureManager::getInstance();

	auto monsterModel = meshMgr->addModel(meshMgr->getResourceAbsolutePath() + "Alien_Animal.fbx", MeshLoadOption::LoadMaterial);
	auto planeMat = matMgr->addMaterial("PlaneMaterial");



	m_scene->addPlane(250, 250, planeMat);
	planeMat->m_shininess = 0.1f;


	auto obj = m_scene->addModel(monsterModel, "Monster");
	obj->m_transform.setScale({ 0.01f, 0.01f, 0.01f });
	obj->m_transform.setPosition({ 0.f, 0.f, 0.f });

	auto camera = m_scene->addCamera({ 0.f, 4.f, 16.f });
	camera->addComponent(ArcballCameraController::create());
	//camera->addComponent(FirstPersonCameraController::create());
	auto cameraController = camera->getComponent<ArcballCameraController>();
	cameraController->setPosition({ 0.f, 4.f, 16.f });

	auto cameraComp = camera->getComponent<CameraComponent>();
	cameraComp->m_fov = 45.f;

	auto dirLight = m_scene->addDirectionalLight({ 0.9f, 0.9f, 0.9f }, 0.9f, ShadowType::SoftShadow);
	dirLight->m_transform.setRotation({ -30.f , -60.f, 0.f });

	
	GuiManager::getInstance()->addWindow(new LightControlGuiWindow("Light Setting", this));
}


void SkeletonAnimationApp::update(double dt) {
	if (!m_scene->isInitialize()) {
		ASSERT(m_scene->initialize());
		ASSERT(GuiManager::getInstance()->initialize());
	}

	m_scene->update(dt);
}


void SkeletonAnimationApp::render() {
	m_renderer->renderScene(m_scene.get());
}

void SkeletonAnimationApp::onWindowResized(int width, int height) {
	__super::onWindowResized(width, height);
	m_scene->onWindowReSize(width, height);
	m_renderer->onWindowResize(width, height);
}
