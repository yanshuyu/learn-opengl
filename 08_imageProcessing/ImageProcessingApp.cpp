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
	std::shared_ptr<MeshRenderComponent> meshRenderer;
	std::shared_ptr<SkinMeshRenderComponent> skinMeshRenderer;
	PhongMaterial* phongMtl;
	PBRMaterial* pbrMtl;
	
	gameObj = m_scene->addModel("phoenix.fbx", MeshLoader::Option::LoadAnimations);
	gameObj->setLayer(SceneLayer::CutOut);
	gameObj->m_transform.setScale({ 0.05f, 0.05f, 0.05f });
	gameObj->m_transform.setPosition({ -25.f, 35.f, 10.f });
	auto animator = gameObj->getComponent<AnimatorComponent>().lock();
	auto idleState = animator->addState("idle");
	idleState->setAnimationClip(animator->animationAt(0));
	idleState->setAnimationLoopType(LoopType::Loop);
	idleState->setAnimationSpeed(0.5);
	skinMeshRenderer = gameObj->getComponent<SkinMeshRenderComponent>().lock();
	skinMeshRenderer->addMaterial(mtlMgr->addMaterial("phoenix_a_mtl", MaterialType::PBR));
	skinMeshRenderer->addMaterial(mtlMgr->addMaterial("phoenix_b_mtl", MaterialType::PBR));
	pbrMtl = skinMeshRenderer->materialAt(0).lock()->asType<PBRMaterial>();
	pbrMtl->m_albedoMap = texMgr->addTexture("Tex_Ride_FengHuang_01a_D_A.tga.png");
	pbrMtl = skinMeshRenderer->materialAt(1).lock()->asType<PBRMaterial>();
	pbrMtl->m_metallic = 0.35;
	pbrMtl->m_roughness = 0.7;
	pbrMtl->m_albedoMap = texMgr->addTexture("Tex_Ride_FengHuang_01b_D_A.tga.png");
	pbrMtl->m_metallic = 0.35;
	pbrMtl->m_roughness = 0.7;


	// PBR monster
	gameObj = m_scene->addModel("Alien_Animal.fbx", MeshLoader::Option::LoadAnimations);
	gameObj->m_transform.setScale({ 0.01f, 0.01f, 0.01f });
	gameObj->m_transform.setPosition({ -15.f, 0.f, 0.f });
	gameObj->setTag(100);
	gameObj->addComponent<AnimatorController>();
	m_animator = gameObj->getComponent<AnimatorComponent>();
	skinMeshRenderer = gameObj->getComponent<SkinMeshRenderComponent>().lock();
	skinMeshRenderer->addMaterial(mtlMgr->addMaterial("monster_xx01_mtl", MaterialType::PBR));
	skinMeshRenderer->addMaterial(mtlMgr->addMaterial("monster_xx02_mtl", MaterialType::PBR));
	skinMeshRenderer->addMaterial(mtlMgr->addMaterial("monster_xx03_mtl", MaterialType::PBR));
	skinMeshRenderer->addMaterial(mtlMgr->addMaterial("monster_eye_mtl", MaterialType::PBR));
	skinMeshRenderer->addMaterial(mtlMgr->addMaterial("monster_body_mtl", MaterialType::PBR));

	pbrMtl = skinMeshRenderer->materialAt(3).lock()->asType<PBRMaterial>();
	pbrMtl->m_albedoMap = texMgr->addTexture("AA_eye Kopie.png");
	pbrMtl->m_mainColor = { 1.f, 1.f, 1.f };
	pbrMtl->m_metallic = 0.1f;
	pbrMtl->m_roughness = 0.9f;

	pbrMtl = skinMeshRenderer->materialAt(4).lock()->asType<PBRMaterial>();
	pbrMtl->m_albedoMap = texMgr->addTexture("monster_albedo.jpg");
	pbrMtl->m_normalMap = texMgr->addTexture("Nor OpenGL.jpg");
	pbrMtl->m_metallicMap = texMgr->addTexture("monster_metallic.jpg");
	pbrMtl->m_roughnessMap = texMgr->addTexture("monster_roughness.jpg");
	pbrMtl->m_mainColor = { 1.f, 1.f, 1.f };
	pbrMtl->m_metallic = 0.9f;
	pbrMtl->m_roughness = 0.2f;

	// PBR man
	gameObj = m_scene->addModel("Mesh_MAT.FBX", MeshLoader::Option::None, "PBR_Man");
	gameObj->m_transform.setPosition({ 15.f, 0.f, 0.f });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("matMan_base_mtl", MaterialType::PBR));
	meshRenderer->addMaterial(mtlMgr->addMaterial("matMan_head_mtl", MaterialType::PBR));
	meshRenderer->addMaterial(mtlMgr->addMaterial("matMan_body_mtl", MaterialType::PBR));
	pbrMtl = meshRenderer->materialAt(0).lock()->asType<PBRMaterial>();
	pbrMtl->m_albedoMap = texMgr->addTexture("03_Base_albedo.jpg");
	pbrMtl->m_normalMap = texMgr->addTexture("03_Base_normal.jpg");
	pbrMtl->m_roughnessMap = texMgr->addTexture("03_Base_roughness.jpg");
	pbrMtl->m_metallicMap = texMgr->addTexture("03_Base_metallic.jpg");
	pbrMtl->m_mainColor = { 1.f, 1.f, 1.f };
	pbrMtl->m_roughness = 1.f;
	pbrMtl->m_metallic = 1.f;

	pbrMtl = meshRenderer->materialAt(1).lock()->asType<PBRMaterial>();
	pbrMtl->m_albedoMap = texMgr->addTexture("01_Head_albedo.jpg");
	pbrMtl->m_normalMap = texMgr->addTexture("01_Head_normal.jpg");
	pbrMtl->m_roughnessMap = texMgr->addTexture("01_Head_roughness.jpg");
	pbrMtl->m_metallicMap = texMgr->addTexture("01_Head_metallic.jpg");
	pbrMtl->m_mainColor = { 1.f, 1.f, 1.f };
	pbrMtl->m_roughness = 1.f;
	pbrMtl->m_metallic = 1.f;

	pbrMtl = meshRenderer->materialAt(2).lock()->asType<PBRMaterial>();
	pbrMtl->m_albedoMap = texMgr->addTexture("02_Body_albedo.jpg");
	pbrMtl->m_normalMap = texMgr->addTexture("02_Body_normal.jpg");
	pbrMtl->m_roughnessMap = texMgr->addTexture("02_Body_roughness.jpg");
	pbrMtl->m_metallicMap = texMgr->addTexture("02_Body_metallic.jpg");
	pbrMtl->m_mainColor = { 1.f, 1.f, 1.f };
	pbrMtl->m_roughness = 1.f;
	pbrMtl->m_metallic = 1.f;


	// terrain
	gameObj = m_scene->addModel("terrain.obj");
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("terrain_mtl", MaterialType::PBR));
	pbrMtl = meshRenderer->materialAt(0).lock()->asType<PBRMaterial>();
	pbrMtl->m_albedoMap = texMgr->addTexture("terrain_diffuse.png");
	//phongMtl->m_specularMap = texMgr->addTexture("terrain_specular.png");
	//phongMtl->m_shininess = 0.9;
	pbrMtl->m_metallic = 0.1f;
	pbrMtl->m_roughness = 1.f;
	pbrMtl->m_mainColor = { 1.f, 1.f, 1.f };

	// brige
	gameObj = m_scene->addModel("bridge.obj");
	gameObj->m_transform.setPosition({ 0, 2, 0 });
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("bridge_mtl", MaterialType::Phong));
	phongMtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	phongMtl->m_albedoMap = texMgr->addTexture("bridge_diffuse.png");
	phongMtl->m_specularColor = { 0, 0, 0 };


	// house
	gameObj = m_scene->addModel("house.obj");
	gameObj->m_transform.setPosition({ 0, 0, 0 });
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("house_mtl", MaterialType::PBR));
	meshRenderer->addMaterial(mtlMgr->getMaterial("house_mtl"));
	pbrMtl = meshRenderer->materialAt(0).lock()->asType<PBRMaterial>();
	pbrMtl->m_albedoMap = texMgr->addTexture("house_diffuse.png");
	//phongMtl->m_specularColor = { 0.1, 0.1, 0.1 };
	pbrMtl->m_metallic = 0.25f;
	pbrMtl->m_roughness = 1.f;
	pbrMtl->m_mainColor = { 1.f, 1.f, 1.f };

	// barrel
	gameObj = m_scene->addModel("barrel.obj");
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("barrel_mtl", MaterialType::Phong));
	phongMtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	phongMtl->m_albedoMap = texMgr->addTexture("barrel_diffuse.png");
	phongMtl->m_shininess = 0.9;
	phongMtl->m_specularColor = { 0.1, 0.1, 0.1 };


	//rocks
	gameObj = m_scene->addModel("rocks.obj");
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("rock_mtl", MaterialType::Phong));
	phongMtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	phongMtl->m_albedoMap = texMgr->addTexture("rocks_diffuse.png");
	phongMtl->m_shininess = 0.9;
	phongMtl->m_specularColor = { 0.1, 0.1, 0.1 };


	//fences
	gameObj = m_scene->addModel("fence.obj");
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("fence_mtl", MaterialType::Phong));
	phongMtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	phongMtl->m_albedoMap = texMgr->addTexture("fence_diffuse.png");
	phongMtl->m_shininess = 0.9;
	phongMtl->m_specularColor = { 0.1, 0.1, 0.1 };


	//path edge
	gameObj = m_scene->addModel("pathedge.obj");
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("pathedge_mtl", MaterialType::Phong));
	phongMtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	phongMtl->m_albedoMap = texMgr->addTexture("pathedge_diffuse.png");
	phongMtl->m_shininess = 0.9;
	phongMtl->m_specularColor = { 0.1, 0.1, 0.1 };


	//lantern
	gameObj = m_scene->addModel("lantern.obj");
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("lantern_mtl", MaterialType::Phong));
	phongMtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	phongMtl->m_albedoMap = texMgr->addTexture("lantern_diffuse.png");
	phongMtl->m_shininess = 0.9;
	phongMtl->m_specularColor = { 0.1, 0.1, 0.1 };


	//flowers
	gameObj = m_scene->addModel("flower.obj");
	gameObj->setLayer(SceneLayer::CutOut);
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("flower_mtl", MaterialType::Phong));
	phongMtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	phongMtl->m_albedoMap = texMgr->addTexture("flower_diffuse.png");
	phongMtl->m_shininess = 0.9;
	phongMtl->m_specularColor = { 0.1, 0.1, 0.1 };

	//mushroom
	gameObj = m_scene->addModel("mushroom.obj");
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("mushroom_mtl", MaterialType::Phong));
	phongMtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	phongMtl->m_albedoMap = texMgr->addTexture("mushroom_diffuse.png");
	phongMtl->m_shininess = 0.9;
	phongMtl->m_specularColor = { 0.1, 0.1, 0.1 };
	

	//trees
	gameObj = m_scene->addModel("tree.obj");
	gameObj->setLayer(SceneLayer::CutOut);
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("tree_mtl", MaterialType::PBR));
	pbrMtl = meshRenderer->materialAt(0).lock()->asType<PBRMaterial>();
	pbrMtl->m_albedoMap = texMgr->addTexture("tree_diffuse.png");
	//phongMtl->m_shininess = 0.9;
	//phongMtl->m_specularColor = { 0.1, 0.1, 0.1 };
	pbrMtl->m_metallic = 0.25f;
	pbrMtl->m_roughness = 1.f;
	pbrMtl->m_mainColor = { 1.f, 1.f, 1.f };

	//cute trees
	gameObj = m_scene->addModel("cutetree.obj");
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("cutetree_mtl", MaterialType::Phong));
	phongMtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	phongMtl->m_albedoMap = texMgr->addTexture("cutetree_diffuse.png");
	phongMtl->m_shininess = 0.9;
	phongMtl->m_specularColor = { 0.1, 0.1, 0.1 };


	//bush
	gameObj = m_scene->addModel("bush.obj");
	gameObj->setLayer(SceneLayer::CutOut);
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("bush_mtl", MaterialType::Phong));
	phongMtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	phongMtl->m_albedoMap = texMgr->addTexture("bush_diffuse.png");
	phongMtl->m_shininess = 0.9;
	phongMtl->m_specularColor = { 0.1, 0.1, 0.1 };


	//bench
	gameObj = m_scene->addModel("bench.obj");
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("bench_mtl", MaterialType::Phong));
	phongMtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	phongMtl->m_albedoMap = texMgr->addTexture("bench_diffuse.png");
	phongMtl->m_shininess = 0.9;
	phongMtl->m_specularColor = { 0.1, 0.1, 0.1 };
	meshRenderer->addMaterial(mtlMgr->getMaterial("bench_mtl"));


	//herbstall
	gameObj = m_scene->addModel("herbstall.obj");
	gameObj->setLayer(SceneLayer::CutOut);
	gameObj->m_transform.setScale({ 10, 10, 10 });
	meshRenderer = gameObj->getComponent<MeshRenderComponent>().lock();
	meshRenderer->clearMaterials();
	meshRenderer->addMaterial(mtlMgr->addMaterial("herbstall_mtl", MaterialType::Phong));
	phongMtl = meshRenderer->materialAt(0).lock()->asType<PhongMaterial>();
	phongMtl->m_albedoMap = texMgr->addTexture("herbstall_diffuse.png");
	phongMtl->m_shininess = 0.9;
	phongMtl->m_specularColor = { 0.1, 0.1, 0.1 };
	
}
