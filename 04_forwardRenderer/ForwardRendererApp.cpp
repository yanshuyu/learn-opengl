#include"ForwardRendererApp.h"
#include<common/ArcballCameraController.h>


ForwardRendererApp::ForwardRendererApp(const std::string& t, int w, int h) :GLApplication(t, w, h)
, m_scene(nullptr)
, m_renderer(nullptr) {

}

bool ForwardRendererApp::initailize() {
	if (!__super::initailize())
		return false;

	auto shaderMgr = ShaderProgramManager::getInstance();
	auto meshMgr = MeshManager::getInstance();
	auto matMgr = MaterialManager::getInstance();
	auto texMgr = TextureManager::getInstance();


	shaderMgr->addProgram(shaderMgr->getResourceAbsolutePath() + "DirectionalLight.shader");
	shaderMgr->addProgram(shaderMgr->getResourceAbsolutePath() + "PointLight.shader");
	shaderMgr->addProgram(shaderMgr->getResourceAbsolutePath() + "Spotlight.shader");
	auto monsterModel = meshMgr->addModel(meshMgr->getResourceAbsolutePath() + "Alien_Animal.fbx", MeshLoadOption::LoadMaterial);
	auto matManModel = meshMgr->addModel(meshMgr->getResourceAbsolutePath() + "Mesh_MAT.FBX", MeshLoadOption::LoadMaterial);
	auto backPackModel = meshMgr->addModel(meshMgr->getResourceAbsolutePath() + "backpack.obj", MeshLoadOption::LoadMaterial);
	auto cubeTexture = texMgr->addTexture(texMgr->getResourceAbsolutePath() + "wall.jpg");
	auto cubeMat = matMgr->addMaterial("CubeMaterial");
	auto planeMat = matMgr->addMaterial("PlaneMaterial");

	m_scene = std::make_unique<Scene>(glm::vec2(m_wndWidth, m_wndHeight), "model_loading_demo_scene");
	m_renderer = std::unique_ptr<Renderer>(new Renderer(m_wndWidth, m_wndHeight, Renderer::Mode::Forward));

	ASSERT(m_renderer->isValid());

	m_scene->addPlane(150, 150, planeMat);
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
	obj->m_transform.setPosition({ -15.f, 0.f, 0.f });
	//obj->m_isVisible = false;

	obj = m_scene->addObject("backPack");
	meshRender = MeshRenderComponent::create();
	meshRender->setMeshes(backPackModel);
	if (!obj->addComponent(meshRender)) {
		MeshRenderComponent::destory(meshRender);
		ASSERT(false);
	}
	obj->m_transform.setPosition({ 0.f, 5.f, 15.f });
	obj->m_transform.setScale({ 2.f, 2.f, 2.f });


	obj = m_scene->addObject("materialMan");
	meshRender = MeshRenderComponent::create();
	meshRender->setMeshes(matManModel);
	if (!obj->addComponent(meshRender)) {
		MeshRenderComponent::destory(meshRender);
		ASSERT(false);
	}
	obj->m_transform.setPosition({ 15.f, 0.f, 0.f });
	meshRender->addMaterial(nullptr);
	meshRender->addMaterial(nullptr);
	meshRender->addMaterial(nullptr);
	auto baseMat = matMgr->addMaterial(obj->getName() + "_Base_Material");
	auto headMat = matMgr->addMaterial(obj->getName() + "_Head_Material");
	auto bodyMat = matMgr->addMaterial(obj->getName() + "Body_Material");
	baseMat->m_diffuseMap = texMgr->addTexture(texMgr->getResourceAbsolutePath() + "03_Base_albedo.jpg");
	headMat->m_diffuseMap = texMgr->addTexture(texMgr->getResourceAbsolutePath() + "01_Head_albedo.jpg");
	bodyMat->m_diffuseMap = texMgr->addTexture(texMgr->getResourceAbsolutePath() + "02_Body_albedo.jpg");
	meshRender->setMaterialAt(0, baseMat);
	meshRender->setMaterialAt(1, headMat);
	meshRender->setMaterialAt(2, bodyMat);

	auto camera = m_scene->addCamera(glm::vec3(0, 4, 16));
	camera->addComponent(FirstPersonCameraController::create());

	auto dirLight =  m_scene->addDirectionalLight({ 0.9f, 0.33f, 0.2f }, 0.4f);
	dirLight->m_transform.setRotation({ -30.f , 30.f, 0.f });

	auto pointLight = m_scene->addPointLight({ 1.f, 1.f, 1.f }, 80, 0.8f);
	pointLight->m_transform.setPosition({ 0.f, 35.f, 25.f });

	auto spotLight = m_scene->addSpotLight({ 1.f, 1.f, 1.f }, 30.f, 60.f, 40.f, 0.8f);
	spotLight->m_transform.setPosition({ 0.f, 20.f, 0.f });
	spotLight->m_transform.setRotation({ -90.f, 0.f, 0.f });

	ASSERT(m_scene->initialize());
	//ASSERT(m_renderer->initialize());
}


void ForwardRendererApp::update(double dt) {
	m_scene->update(dt);
}


void ForwardRendererApp::render() {
	m_renderer->renderScene(m_scene.get());
}

void ForwardRendererApp::onWindowResized(int width, int height) {
	__super::onWindowResized(width, height);
	m_scene->onWindowReSize(width, height);
	m_renderer->onWindowResize(width, height);
}
