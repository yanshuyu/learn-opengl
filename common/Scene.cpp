#include"Scene.h"
#include"InputMgr.h"
#include"MeshMgr.h"
#include"NotificationCenter.h"
#include"LightComponent.h"
#include"AnimatorComponent.h"
#include"SkinMeshRenderComponent.h"
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<algorithm>


Scene::Scene(const glm::vec2& wndSz, const std::string& name): m_name(name)
, m_isInitialize(false)
, m_id(0)
, m_windowSize(wndSz)
, m_defaultCamera(wndSz.x/wndSz.y) 
, m_activeCamera(nullptr) {
	m_id = reinterpret_cast<unsigned long>(this);
	m_rootObject = std::make_unique<SceneObject>("root");
}


Scene::~Scene() {
}


bool Scene::initialize() {
	if (m_isInitialize)
		return true;
	
	m_isInitialize = m_rootObject->initialize();

	return m_isInitialize;
}


void Scene::update(double dt) {
	if (!m_isInitialize)
		if (!initialize()) {
#ifdef _DEBUG
			std::cerr << "Scene \"" << m_name << "(" << m_id << ")\": failed to init!" << std::endl;
#endif // _DEBUG
			return;
		}

	m_rootObject->update(dt);
}


SceneRenderInfo_t* Scene::getSceneRenderInfo() const {
	static SceneRenderInfo_t sri;
	CameraComponent* activeCamera = nullptr;
	sri.lights.clear();

	depthFirstVisit([&](SceneObject* obj, bool& stop) -> bool {
		for (size_t i = 0; i < obj->componentCount(); i++) {
			Component* comp = obj->componentAt(i);
			
			if (!comp->m_isEnable)
				continue;

			if (comp->identifier() == CameraComponent::s_identifier && !activeCamera) {
				activeCamera = static_cast<CameraComponent*>(comp);
				sri.camera = activeCamera->makeCamera(m_windowSize.x, m_windowSize.y);
			}

			if (comp->identifier() == LightComponent::s_identifier)
				sri.lights.push_back(static_cast<LightComponent*>(comp)->makeLight());
		}
		
		return true;
	});

	if (activeCamera) 
		activeCamera->m_aspectRatio = m_windowSize.x / m_windowSize.y;
	
	if (!activeCamera)
		sri.camera = m_defaultCamera.makeCamera(m_windowSize.x, m_windowSize.y);
	
	m_activeCamera = &sri.camera;

	return &sri;
}

void Scene::render(RenderContext* context) {
	m_rootObject->render(context);
}


SceneObject* Scene::addObject(const std::string& name) {
	SceneObject* obj = new SceneObject(name);
	m_rootObject->addChild(obj);
	return obj;
}

void Scene::addOject(SceneObject* object) {
	m_rootObject->addChild(object);
}

void Scene::addObject(std::unique_ptr<SceneObject>&& object) {
	m_rootObject->addChild(std::move(object));
}

SceneObject* Scene::addCamera(const glm::vec3& p, const glm::vec3& r, const glm::vec3& bgColor) {
	SceneObject* camera = addObject("Camera");
	CameraComponent* cameraComp = new CameraComponent(m_windowSize.x / m_windowSize.y);
	camera->addComponent(cameraComp);
	camera->m_transform.setPosition(p);
	camera->m_transform.setRotation(r);
	cameraComp->m_backGroundColor = glm::vec4(bgColor, 1.f);
	camera->setTag(Tag::Camera);

	return camera;
}

SceneObject* Scene::addModel(const std::string& file, const std::string& name) {
	auto model = MeshManager::getInstance()->addMesh(file).lock();

	std::string modelName = name.empty() ? model->getName() : name;
	SceneObject* obj = addObject(modelName);
	
	if (model->hasAnimation()) {
		SkinMeshRenderComponent* meshRender = obj->addComponent<SkinMeshRenderComponent>();
		meshRender->setMeshes(model);

		AnimatorComponent* animator = obj->addComponent<AnimatorComponent>();
		animator->setAvater(model);

		meshRender->setAnimator(obj->getComponent<AnimatorComponent>());
	}
	else {
		MeshRenderComponent* meshRender = obj->addComponent<MeshRenderComponent>();
		meshRender->setMeshes(model);
	}


	return obj;
}

SceneObject* Scene::addGrid(float w, float d, float spacing, std::shared_ptr<Material> mat) {
	SceneObject* grid = addObject("Grid");
	auto gridMesh = MeshManager::getInstance()->createGrid(w, d, spacing);
	auto meshRenderComp = MeshRenderComponent::create();
	meshRenderComp->setMeshes(gridMesh);
	grid->addComponent(meshRenderComp);

	if (mat)
		meshRenderComp->addMaterial(mat);
	
	return grid;
}


SceneObject* Scene::addPlane(float w, float d, std::shared_ptr<Material> mat) {
	SceneObject* plane = addObject("Plane");
	auto planeMesh = MeshManager::getInstance()->createPlane(w, d);
	auto meshRenderComp = MeshRenderComponent::create();
	meshRenderComp->setMeshes(planeMesh);
	plane->addComponent(meshRenderComp);

	if (mat)
		meshRenderComp->addMaterial(mat);

	return nullptr;
}


SceneObject* Scene::addCube(std::shared_ptr<Material> mat) {
	SceneObject* cube = addObject("Cube");
	auto cubeMesh = MeshManager::getInstance()->createCube();
	auto meshRenderComp = MeshRenderComponent::create();
	meshRenderComp->setMeshes(cubeMesh);
	cube->addComponent(meshRenderComp);

	if (mat)
		meshRenderComp->addMaterial(mat);

	return  cube;
}


SceneObject* Scene::addDirectionalLight(const glm::vec3& color, float intensity, ShadowType shadowType) {
	auto light = addObject("DirectionalLight");
	auto lightComp = LightComponent::create();
	lightComp->setType(LightType::DirectioanalLight);
	lightComp->setColor(color);
	lightComp->setIntensity(intensity);
	lightComp->setShadowType(shadowType);
	light->addComponent(lightComp);
	light->setTag(Tag::DirectionalLight);

	return light;
}

SceneObject* Scene::addPointLight(const glm::vec3& color, float range, float intensity, ShadowType shadowType) {
	auto light = addObject("PointLight");
	auto lightComp = LightComponent::create();
	lightComp->setType(LightType::PointLight);
	lightComp->setColor(color);
	lightComp->setRange(range);
	lightComp->setIntensity(intensity);
	lightComp->setShadowType(shadowType);
	light->addComponent(lightComp);
	light->setTag(PointLight);

	return light;
}

SceneObject* Scene::addSpotLight(const glm::vec3& color, float innerAngle, float outterAngle, float range, float intensity, ShadowType shadowType, float shadowStrength) {
	auto light = addObject("SpotLight");
	auto lightComp = LightComponent::create();
	lightComp->setType(LightType::SpotLight);
	lightComp->setColor(color);
	lightComp->setRange(range);
	lightComp->setIntensity(intensity);
	lightComp->setSpotInnerAngle(innerAngle);
	lightComp->setSpotOutterAngle(outterAngle);
	lightComp->setShadowType(shadowType);
	lightComp->setShadowStrength(shadowStrength);
	light->addComponent(lightComp);
	light->setTag(Tag::SpotLight);

	return light;
}

SceneObject* Scene::findObjectWithID(ID id) const {
	return m_rootObject->findChildWithID(id);
}

SceneObject* Scene::findObjectWithTag(ID tag) const {
	return m_rootObject->findChildWithTag(tag);
}

SceneObject* Scene::findObjectWithName(const std::string& name) const {
	return m_rootObject->findChildWithName(name);
}

SceneObject* Scene::findObjectWithTagRecursive(ID tag) const {
	return m_rootObject->findChildWithTagRecursive(tag);
}

SceneObject* Scene::findObjectWithNameRecursive(const std::string& name) const {
	return m_rootObject->findChildWithNameRecursive(name);
}

std::unique_ptr<SceneObject> Scene::removeObjectWithID(ID id) {
	return m_rootObject->removeChildWithID(id);
}

std::unique_ptr<SceneObject> Scene::removeObjectWithTag(ID tag) {
	return m_rootObject->removeChildWithTag(tag);
}

std::unique_ptr<SceneObject> Scene::removeObjectWithName(const std::string& name) {
	return m_rootObject->removeChildWithName(name);
}


void Scene::clearObjects() {
	m_rootObject->clearChilds();
}


size_t Scene::objectCount() const {
	return m_rootObject->childCount();
}

size_t Scene::objectCountRecursive() const {
	return m_rootObject->childCountRecursive();
}


void Scene::depthFirstVisit(std::function<bool(SceneObject*, bool&)> op) const {
	m_rootObject->depthFirstTraverse(op);
}


void Scene::breathFirstVisit(std::function<bool(SceneObject*, bool&)> op) const {
	m_rootObject->breadthFirstTraverse(op);
}
