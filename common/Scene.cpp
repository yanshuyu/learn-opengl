#include"Scene.h"
#include"InputMgr.h"
#include"NotificationCenter.h"
#include"CameraComponent.h"
#include<glm/gtc/matrix_transform.hpp>


Scene::Scene(const glm::vec2& wndSz, const std::string& name): m_name(name)
, m_isInitialize(false)
, m_id(0)
, m_windowSize(wndSz){
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


void Scene::preRender(Renderer* renderer) {

}

void Scene::render(RenderContext* context) {
	m_rootObject->render(context);
}

void Scene::afterRender(Renderer* renderer) {

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

SceneObject* Scene::addCamera(const glm::vec3& p, const glm::vec3& r) {
	SceneObject* camera = new SceneObject("Camera");
	CameraComponent* cameraComp = new CameraComponent(m_windowSize.x, m_windowSize.y);
	camera->addComponent(cameraComp);
	camera->m_transform.setPosition(p);
	camera->m_transform.setRotation(r);
	addObject(std::unique_ptr<SceneObject>(camera));

	return camera;
}


CameraComponent* Scene::getCamera() const {
	CameraComponent* cameraComp = nullptr;
	breathFirstVisit([&](const SceneObject* obj, bool& stop) {
		if (auto c = obj->findComponent(CameraComponent::s_indentifier)) {
			if (c->m_isEnable) {
				cameraComp = static_cast<CameraComponent*>(c);
				stop = true;
				return false;
			}
		}
		return true;
	});
	return cameraComp;
}

SceneObject* Scene::findObjectWithID(ID id) const {
	return m_rootObject->findChildWithID(id);
}

SceneObject* Scene::findObjectWithTag(ID tag) const {
	return findObjectWithTag(tag);
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