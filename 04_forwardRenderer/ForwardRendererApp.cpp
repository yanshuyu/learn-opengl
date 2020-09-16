#include"ForwardRendererApp.h"
#include<common/ArcballCameraController.h>


ForwardRendererApp::ForwardRendererApp(const std::string& t, int w, int h) :GLApplication(t, w, h)
, m_scene(nullptr)
, m_renderer(nullptr) {

}

bool ForwardRendererApp::initailize() {
	if (!__super::initailize())
		return false;

	auto meshMgr = MeshManager::getInstance();
	auto matMgr = MaterialManager::getInstance();
	auto texMgr = TextureManager::getInstance();

	auto monsterModel = meshMgr->addModel(meshMgr->getResourceAbsolutePath() + "Alien_Animal.fbx", MeshLoadOption::LoadMaterial);
	auto matManModel = meshMgr->addModel(meshMgr->getResourceAbsolutePath() + "Mesh_MAT.FBX", MeshLoadOption::LoadMaterial);
	auto cubeTexture = texMgr->addTexture(texMgr->getResourceAbsolutePath() + "wall.jpg");

	m_scene = std::make_unique<Scene>(glm::vec2(m_wndWidth, m_wndHeight), "model_loading_demo_scene");
	m_renderer = std::unique_ptr<Renderer>(new Renderer(new ForwardRenderer()));

	//m_scene->addGrid(300, 300, 10);

	m_scene->addPlane(150, 150);

	auto cubeMat = matMgr->addMaterial("CubeMaterial");
	cubeMat->m_diffuseMap = cubeTexture;

	auto cube = m_scene->addCube(cubeMat);
	cube->m_transform.setScale({ 4.f, 4.f, 4.f });

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

	ASSERT(m_scene->initialize());
	ASSERT(m_renderer->initialize());
}


void ForwardRendererApp::update(double dt) {
	m_scene->update(dt);
}


void ForwardRendererApp::render() {
	m_renderer->renderScene(m_scene.get());
}

void ForwardRendererApp::onWindowResized(int width, int height) {
	__super::onWindowResized(width, height);
	m_scene->setWindowSize(width, height);
}
