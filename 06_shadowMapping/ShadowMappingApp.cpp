#include"ShadowMappingApp.h"
#include"LightControlGuiWindow.h"


ShadowMappingApp::ShadowMappingApp(const std::string& t, int w, int h) :GLApplication(t, w, h)
, m_scene(nullptr)
, m_renderer(nullptr) {

}

bool ShadowMappingApp::initailize() {
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
	auto matManModel = meshMgr->addModel(meshMgr->getResourceAbsolutePath() + "Mesh_MAT.FBX", MeshLoadOption::LoadMaterial);
	auto backPackModel = meshMgr->addModel(meshMgr->getResourceAbsolutePath() + "backpack.obj", MeshLoadOption::LoadMaterial);
	auto cubeTexture = texMgr->addTexture(texMgr->getResourceAbsolutePath() + "wall.jpg");
	auto cubeMat = matMgr->addMaterial("CubeMaterial");
	auto planeMat = matMgr->addMaterial("PlaneMaterial");



	m_scene->addPlane(250, 250, planeMat);
	planeMat->m_shininess = 0.1f;


	auto cube = m_scene->addCube(cubeMat);
	cube->m_transform.setScale({ 4.f, 4.f, 4.f });
	cubeMat->m_diffuseMap = cubeTexture;
	cubeMat->m_shininess = 0.1f;

	auto obj = m_scene->addObject("monster");
	auto meshRender = MeshRenderComponent::create();
	meshRender->setMeshes(monsterModel);
	if (!obj->addComponent(meshRender)) {
		MeshRenderComponent::destory(meshRender);
		ASSERT(false);
	}
	obj->m_transform.setScale({ 0.01f, 0.01f, 0.01f });
	obj->m_transform.setPosition({ -20.f, 0.f, 0.f });
	//obj->m_isVisible = false;

	obj = m_scene->addObject("backPack");
	meshRender = MeshRenderComponent::create();
	meshRender->setMeshes(backPackModel);
	if (!obj->addComponent(meshRender)) {
		MeshRenderComponent::destory(meshRender);
		ASSERT(false);
	}
	meshRender->materialAt(0)->m_normalMap = texMgr->addTexture(texMgr->getResourceAbsolutePath() + "normal.png");
	obj->m_transform.setPosition({ 0.f, 5.f, 20.f });
	obj->m_transform.setScale({ 2.f, 2.f, 2.f });


	obj = m_scene->addObject("materialMan");
	meshRender = MeshRenderComponent::create();
	meshRender->setMeshes(matManModel);
	if (!obj->addComponent(meshRender)) {
		MeshRenderComponent::destory(meshRender);
		ASSERT(false);
	}
	obj->m_transform.setPosition({ 20.f, 0.f, 0.f });
	auto baseMat = meshRender->materialAt(0);
	auto headMat = meshRender->materialAt(1);
	auto bodyMat = meshRender->materialAt(2);
	baseMat->m_diffuseMap = texMgr->addTexture(texMgr->getResourceAbsolutePath() + "03_Base_albedo.jpg");
	baseMat->m_normalMap = texMgr->addTexture(texMgr->getResourceAbsolutePath(), + "03_Base_normal.jpg");
	
	headMat->m_diffuseMap = texMgr->addTexture(texMgr->getResourceAbsolutePath() + "01_Head_albedo.jpg");
	headMat->m_normalMap = texMgr->addTexture(texMgr->getResourceAbsolutePath() + "01_Head_normal.jpg");
	headMat->m_emissiveMap = texMgr->addTexture(texMgr->getResourceAbsolutePath() + "01_Head_emissive.jpg");
	headMat->m_emissiveColor = { 1.f, 1.f, 1.f };

	bodyMat->m_diffuseMap = texMgr->addTexture(texMgr->getResourceAbsolutePath() + "02_Body_albedo.jpg");
	bodyMat->m_normalMap = texMgr->addTexture(texMgr->getResourceAbsolutePath() + "02_Body_normal.jpg");


	auto camera = m_scene->addCamera({ 0.f, 4.f, 16.f });
	camera->addComponent(ArcballCameraController::create());
	//camera->addComponent(FirstPersonCameraController::create());
	auto cameraController = camera->getComponent<ArcballCameraController>();
	cameraController->setPosition({ 0.f, 4.f, 16.f });

	auto cameraComp = camera->getComponent<CameraComponent>();
	cameraComp->m_fov = 45.f;

	auto dirLight = m_scene->addDirectionalLight({ 0.9f, 0.9f, 0.9f }, 0.9f, ShadowType::SoftShadow);
	dirLight->m_transform.setRotation({ -30.f , -60.f, 0.f });


	//auto pointLight = m_scene->addPointLight({ 1.f, 1.f, 1.f }, 80, 1.f, ShadowType::SoftShadow);
	//pointLight->m_transform.setPosition({ 10.f, 30.f, 0.f });

	//auto spotLight = m_scene->addSpotLight({ 1.f, 1.f, 1.f }, 60.f, 65.f, 100.f, 1.f, ShadowType::SoftShadow);
	//spotLight->m_transform.setPosition({ 0.f, 25.f, 8.f });
	//spotLight->m_transform.setRotation({ -65.f, 0.f, 0.f });


	GuiManager::getInstance()->addWindow(new LightControlGuiWindow("Shadow demo", this));

}


void ShadowMappingApp::update(double dt) {
	if (!m_scene->isInitialize()) {
		ASSERT(m_scene->initialize());
		ASSERT(GuiManager::getInstance()->initialize());
	}

	m_scene->update(dt);
}


void ShadowMappingApp::render() {
	m_renderer->renderScene(m_scene.get());
}

void ShadowMappingApp::onWindowResized(int width, int height) {
	__super::onWindowResized(width, height);
	m_scene->onWindowReSize(width, height);
	m_renderer->onWindowResize(width, height);
}
