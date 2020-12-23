#include"Scene.h"
#include"InputMgr.h"
#include"MeshMgr.h"
#include"NotificationCenter.h"
#include"LightComponent.h"
#include"CameraComponent.h"
#include"AnimatorComponent.h"
#include"SkinMeshRenderComponent.h"
#include"Renderer.h"
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<algorithm>


Scene::Scene(const std::string& name): m_name(name)
, m_isInitialize(false)
, m_id(0)
, m_lights()
, m_cameras()
, m_mainCamera()
, m_renderContext() {
	m_id = reinterpret_cast<unsigned long>(this);
	m_rootObject = std::make_unique<SceneObject>("root");
	m_rootObject->m_parentScene = this;
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


glm::vec2 Scene::getRenderSize() const {
	if (auto renderer = getRenderer())
		return renderer->getRenderSize();

	return glm::vec2(0.f);
}


void Scene::setEnviromentLight(const glm::vec3& sky, const glm::vec3& ground) {
	m_ambientSky = sky;
	m_ambientGround = ground;
	if (auto renderer = getRenderer())
		renderer->setEnviromentLight(m_ambientSky, m_ambientGround);
}


void Scene::render() {
	Renderer* renderer = m_renderContext.getRenderer();
	if (!renderer) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return;
	}

	for (auto& camera : m_cameras) {
		if (!camera->m_isEnable)
			continue;
		renderer->submitCamera(camera->makeCamera(renderer->getRenderSize()), camera == m_mainCamera);
	}

	for (auto& light : m_lights) {
		if (!light->m_isEnable)
			continue;
		renderer->submitLight(light->makeLight());
	}

	m_renderContext.clearMatrix();
	m_renderContext.pushMatrix(glm::mat4(1.f));
	m_rootObject->render(&m_renderContext);

	renderer->flush();
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

SceneObject* Scene::addCamera(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& bgColor) {
	auto renderSize = getRenderSize();
	ASSERT(renderSize.y != 0);

	SceneObject* camera = addObject("Camera");
	CameraComponent* cameraComp = new CameraComponent(renderSize.x / renderSize.y);
	camera->m_transform.setPosition(pos);
	camera->m_transform.setRotation(rot);
	cameraComp->m_backGroundColor = glm::vec4(bgColor, 1.f);
	camera->setTag(Tag::Camera);
	camera->addComponent(cameraComp);

	return camera;
}

SceneObject* Scene::addModel(const std::string& file, int loadOptions, const std::string& name) {
	auto model = MeshManager::getInstance()->addMesh(file, loadOptions).lock();

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

SceneObject* Scene::addGrid(float w, float d, float spacing, std::weak_ptr<IMaterial> mat) {
	SceneObject* grid = addObject("Grid");
	auto gridMesh = MeshManager::getInstance()->createGrid(w, d, spacing);
	auto meshRenderComp = MeshRenderComponent::create();
	meshRenderComp->setMeshes(gridMesh);
	grid->addComponent(meshRenderComp);

	if (!mat.expired())
		meshRenderComp->addMaterial(mat);
	
	return grid;
}


SceneObject* Scene::addPlane(float w, float d, std::weak_ptr<IMaterial> mat) {
	SceneObject* plane = addObject("Plane");
	auto planeMesh = MeshManager::getInstance()->createPlane(w, d);
	auto meshRenderComp = MeshRenderComponent::create();
	meshRenderComp->setMeshes(planeMesh);
	plane->addComponent(meshRenderComp);

	if (!mat.expired())
		meshRenderComp->addMaterial(mat);

	return plane;
}


SceneObject* Scene::addCube(std::weak_ptr<IMaterial> mat) {
	SceneObject* cube = addObject("Cube");
	auto cubeMesh = MeshManager::getInstance()->createCube();
	auto meshRenderComp = MeshRenderComponent::create();
	meshRenderComp->setMeshes(cubeMesh);
	cube->addComponent(meshRenderComp);

	if (!mat.expired())
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
	m_rootObject->removeAllChildren();
}


size_t Scene::objectCount() const {
	return m_rootObject->childCount();
}

size_t Scene::objectCountRecursive() const {
	return m_rootObject->childCountRecursive();
}

void Scene::onWindowResize(float width, float height) {
	for (auto camera : m_cameras) {
		camera->onWindowResized({width, height});
	}
}

void Scene::onCameraAdded(SceneObject* obj, CameraComponent* camera) {
	m_cameras.push_back(camera);
	if (!m_mainCamera)
		m_mainCamera = camera;
}

void Scene::onCameraRemoved(SceneObject* obj, CameraComponent* camera) {
	auto pos = std::remove(m_cameras.begin(), m_cameras.end(), camera);
	if (pos != m_cameras.end()) {
		m_cameras.erase(pos);
		if (camera == m_mainCamera) {
			m_mainCamera = nullptr;
			if (m_cameras.size() > 0)
				m_mainCamera = m_cameras.back();
		}
	}
}

void Scene::onLightAdded(SceneObject* obj, LightComponent* light) {
	m_lights.push_back(light);
}

void Scene::onLightRemoved(SceneObject* obj, LightComponent* light) {
	auto pos = std::remove(m_lights.begin(), m_lights.end(), light);
	if (pos != m_lights.end())
		m_lights.erase(pos);
}


void Scene::depthFirstVisit(std::function<bool(SceneObject*, bool&)> op) const {
	m_rootObject->depthFirstTraverse(op);
}


void Scene::breathFirstVisit(std::function<bool(SceneObject*, bool&)> op) const {
	m_rootObject->breadthFirstTraverse(op);
}


