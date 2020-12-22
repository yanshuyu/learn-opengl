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
	m_scene->setEnviromentLight({ 0.f, 0.f, 0.1f }, { 0.f, 0.1f, 0.f });

	_loadScene();
	
	auto shaderMgr = ShaderProgramManager::getInstance();
	auto meshMgr = MeshManager::getInstance();
	auto matMgr = MaterialManager::getInstance();
	auto texMgr = TextureManager::getInstance();
	
	auto myshader = shaderMgr->addProgram("DirectionalLight");

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
	//camera->addComponent<GrayFilterComponent>();
	camera->addComponent<HDRFilterComponent>();
	//camera->addComponent<GaussianBlurFilterComponent>();

	auto skyBox = camera->addComponent<SkyboxComponent>();
	ASSERT(skyBox->load("sky_right.png", "sky_left.png", "sky_top.png", "sky_bottom.png", "sky_front.png", "sky_back.png"));


	//camera->addComponent(ArcballCameraController::create());
	//auto cameraController = camera->getComponent<ArcballCameraController>();
	//cameraController.lock()->setPosition({ 0.f, 4.f, 16.f });

	auto pointLight = m_scene->addPointLight({ 1.f, 1.f, 0.8f }, 80.f, 1.f, ShadowType::NoShadow);
	pointLight->m_transform.setPosition({-20.f, 40.f, -10.f});

	auto spotLight = m_scene->addSpotLight({ 1.f, 1.f, 0.8f }, 45.f, 65.f, 80.f, 1.f, ShadowType::NoShadow);
	spotLight->m_transform.setPosition({ -15.f, 40.f, 10.f });
	spotLight->m_transform.setRotation({ -45.f, 0.f, 0.f });
	
	// light
	auto dirLight = m_scene->addDirectionalLight({ 0.9f, 0.9f, 0.9f }, 0.9f, ShadowType::SoftShadow);
	dirLight->m_transform.setRotation({ -30.f , -60.f, 0.f });

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



void ImageProcessingApp::_loadScene() {
	auto meshMgr = MeshManager::getInstance();
	auto texMgr = TextureManager::getInstance();
	auto mtlMgr = MaterialManager::getInstance();

	SceneObject* gameObj;
	PhongMaterial* mtl;

	// terrain
	gameObj = m_scene->addModel("terrain.obj");
	gameObj->m_transform.setScale({ 10, 10, 10 });
	auto meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("terrain_mtl", MaterialType::Phong));
	mtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	mtl->m_albedoMap = texMgr->addTexture("terrain_diffuse.png");
	mtl->m_specularMap = texMgr->addTexture("terrain_specular.png");
	mtl->m_shininess = 0.9;
	
	// brige
	gameObj = m_scene->addModel("bridge.obj");
	gameObj->m_transform.setPosition({ 0, 2, 0 });
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("bridge_mtl", MaterialType::Phong));
	mtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	mtl->m_albedoMap = texMgr->addTexture("bridge_diffuse.png");
	mtl->m_specularColor = { 0, 0, 0 };


	// house
	gameObj = m_scene->addModel("house.obj");
	gameObj->m_transform.setPosition({ 0, 0, 0 });
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("house_mtl", MaterialType::Phong));
	meshRenderer->addMaterial(mtlMgr->getMaterial("house_mtl"));
	mtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	mtl->m_albedoMap = texMgr->addTexture("house_diffuse.png");
	mtl->m_specularColor = { 0.1, 0.1, 0.1 };


	// barrel
	gameObj = m_scene->addModel("barrel.obj");
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("barrel_mtl", MaterialType::Phong));
	mtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	mtl->m_albedoMap = texMgr->addTexture("barrel_diffuse.png");
	mtl->m_shininess = 0.9;
	mtl->m_specularColor = { 0.1, 0.1, 0.1 };


	//rocks
	gameObj = m_scene->addModel("rocks.obj");
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("rock_mtl", MaterialType::Phong));
	mtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	mtl->m_albedoMap = texMgr->addTexture("rocks_diffuse.png");
	mtl->m_shininess = 0.9;
	mtl->m_specularColor = { 0.1, 0.1, 0.1 };


	//fences
	gameObj = m_scene->addModel("fence.obj");
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("fence_mtl", MaterialType::Phong));
	mtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	mtl->m_albedoMap = texMgr->addTexture("fence_diffuse.png");
	mtl->m_shininess = 0.9;
	mtl->m_specularColor = { 0.1, 0.1, 0.1 };


	//path edge
	gameObj = m_scene->addModel("pathedge.obj");
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("pathedge_mtl", MaterialType::Phong));
	mtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	mtl->m_albedoMap = texMgr->addTexture("pathedge_diffuse.png");
	mtl->m_shininess = 0.9;
	mtl->m_specularColor = { 0.1, 0.1, 0.1 };


	//lantern
	gameObj = m_scene->addModel("lantern.obj");
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("lantern_mtl", MaterialType::Phong));
	mtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	mtl->m_albedoMap = texMgr->addTexture("lantern_diffuse.png");
	mtl->m_shininess = 0.9;
	mtl->m_specularColor = { 0.1, 0.1, 0.1 };


	//flowers
	gameObj = m_scene->addModel("flower.obj");
	gameObj->setLayer(SceneLayer::CutOut);
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("flower_mtl", MaterialType::Phong));
	mtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	mtl->m_albedoMap = texMgr->addTexture("flower_diffuse.png");
	mtl->m_shininess = 0.9;
	mtl->m_specularColor = { 0.1, 0.1, 0.1 };

	//mushroom
	gameObj = m_scene->addModel("mushroom.obj");
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("mushroom_mtl", MaterialType::Phong));
	mtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	mtl->m_albedoMap = texMgr->addTexture("mushroom_diffuse.png");
	mtl->m_shininess = 0.9;
	mtl->m_specularColor = { 0.1, 0.1, 0.1 };
	

	//trees
	gameObj = m_scene->addModel("tree.obj");
	gameObj->setLayer(SceneLayer::CutOut);
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("tree_mtl", MaterialType::Phong));
	mtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	mtl->m_albedoMap = texMgr->addTexture("tree_diffuse.png");
	mtl->m_shininess = 0.9;
	mtl->m_specularColor = { 0.1, 0.1, 0.1 };


	//cute trees
	gameObj = m_scene->addModel("cutetree.obj");
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("cutetree_mtl", MaterialType::Phong));
	mtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	mtl->m_albedoMap = texMgr->addTexture("cutetree_diffuse.png");
	mtl->m_shininess = 0.9;
	mtl->m_specularColor = { 0.1, 0.1, 0.1 };


	//bush
	gameObj = m_scene->addModel("bush.obj");
	gameObj->setLayer(SceneLayer::CutOut);
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("bush_mtl", MaterialType::Phong));
	mtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	mtl->m_albedoMap = texMgr->addTexture("bush_diffuse.png");
	mtl->m_shininess = 0.9;
	mtl->m_specularColor = { 0.1, 0.1, 0.1 };


	//bench
	gameObj = m_scene->addModel("bench.obj");
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("bench_mtl", MaterialType::Phong));
	mtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	mtl->m_albedoMap = texMgr->addTexture("bench_diffuse.png");
	mtl->m_shininess = 0.9;
	mtl->m_specularColor = { 0.1, 0.1, 0.1 };
	meshRenderer->addMaterial(mtlMgr->getMaterial("bench_mtl"));


	//herbstall
	gameObj = m_scene->addModel("herbstall.obj");
	gameObj->setLayer(SceneLayer::CutOut);
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("herbstall_mtl", MaterialType::Phong));
	mtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	mtl->m_albedoMap = texMgr->addTexture("herbstall_diffuse.png");
	mtl->m_shininess = 0.9;
	mtl->m_specularColor = { 0.1, 0.1, 0.1 };
	
}
