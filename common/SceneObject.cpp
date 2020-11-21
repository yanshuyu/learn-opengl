#include"SceneObject.h"
#include"Renderer.h"
#include"RenderableComponent.h"
#include<stack>
#include<queue>

#define SAFE_UNWRAP_OBJECT_ITERATOR(itr)  itr == m_childs.end() ? nullptr : itr->get();


SceneObject::SceneObject(const std::string& name) :m_name(name)
, m_id(0)
, m_transform(this)
, m_isEnable(true)
, m_isVisible(true)
, m_parent(nullptr)
, m_tag(0)
, m_layer(Layer::Default)
, m_components()
, m_parentScene() {
	m_id = reinterpret_cast<ID>(this);
}


SceneObject::~SceneObject() {
	removeAllComponent();
	m_parentScene = nullptr;
}


SceneObject* SceneObject::copy() const {
	SceneObject* copyed = new SceneObject(m_name + " Copy");
	copyed->m_transform = m_transform;
	copyed->m_isEnable = m_isEnable;
	copyed->m_isVisible = m_isVisible;
	for (auto c = m_components.begin(); c != m_components.end(); c++) {
		Component* cc = (*c)->copy();
		copyed->addComponent(cc);
	}

	for (size_t i = 0; i < m_childs.size(); i++) {
		SceneObject* copyedChild = m_childs[i]->copy();
		copyed->addChild(copyedChild);
	}

	return copyed;
}

bool SceneObject::initialize() {
	bool success = true;
	depthFirstTraverse([&](SceneObject* obj, bool& stop) {
		for (size_t i = 0; i < obj->componentCount(); i++) {
			auto c = obj->componentAt(i);
			if (c->m_isEnable ) {
				bool ok = c->initialize();
				if (!ok) {
					success = false;
					std::cerr << "Game object \"" << obj->getName() << "\" Failed to initailize Component \"" << c->typeName() << "\"" << std::endl;
				}
#ifdef _DEBUG
				ASSERT(ok);
#endif // _DEBUG
			}
		}
		return true;
	});

	return success;
}

void SceneObject::update(double dt) {
	depthFirstTraverse([=](SceneObject* obj, bool& stop) {
		if (!obj->m_isEnable)
			return false;
		for (size_t i = 0; i < obj->componentCount(); i++) {
			auto c = obj->componentAt(i);
			
			if (!c->m_isEnable)
				continue;

			c->update(dt);
		}
		return true;
	});
}

void SceneObject::render(RenderContext* context) const {
	if (!m_isVisible)
		return;

	for (auto& c : m_components) {
		if (auto drawable = c->asType<RenderableComponent>()) {
			if (drawable->m_isEnable)
				drawable->render(context);
		}
	}

	context->pushMatrix(m_transform.getMatrix());
	for (size_t i = 0; i < m_childs.size(); i++) {
		m_childs[i]->render(context);
	}
	context->popMatrix();
}

bool SceneObject::addComponent(Component* c) {
	return addComponent(std::shared_ptr<Component>(c));
}

bool SceneObject::addComponent(std::shared_ptr<Component> c) {
	if (!c)
		return false;
	
	if (c->m_owner)
		return false;

	if (hasComponent(c->typeId()))
		return false;
	
	c->m_owner = this;
	m_components.push_back(c);

	c->onAttached();

	return true;
}

bool SceneObject::hasComponent(const ID id) const {
	for (auto& c : m_components) {
		if (c->typeId() == id)
			return true;
	}

	return false;
}

std::weak_ptr<Component> SceneObject::getComponent(const ID id) const {
	for (auto& c : m_components) {
		if (c->typeId() == id)
			return c;
	}

	return std::weak_ptr<Component>();
}


bool SceneObject::removeComponent(const ID id) {
	for (auto itr = m_components.begin(); itr != m_components.end(); itr++) {
		if ((*itr)->typeId() == id) {
			m_components.erase(itr);
			(*itr)->onDetached();

			return true;
		}
	}

	return false;
}


void SceneObject::removeAllComponent() {
	for (auto& c : m_components) {
		c->onDetached();
	}
	m_components.clear();
}


void SceneObject::addChild(SceneObject* c) {
	addChild(std::unique_ptr<SceneObject>(c));
}

void SceneObject::addChild(std::unique_ptr<SceneObject>&& c) {
	auto _c = c.get();
	m_childs.push_back(std::move(c));
	_c->m_parent = this;
	_c->m_parentScene = m_parentScene;
}

void SceneObject::insertChild(SceneObject* c, size_t index) {
	insertChild(std::unique_ptr<SceneObject>(c), index);
}

void SceneObject::insertChild(std::unique_ptr<SceneObject>&& c, size_t index) {
	auto _c = c.get();
	m_childs.insert(m_childs.begin() + index, std::move(c));
	_c->m_parent = this;
	_c->m_parentScene = m_parentScene;
}

void SceneObject::movechild(size_t srcIndex, size_t dstIndex) {
	if (srcIndex > dstIndex) {
		std::unique_ptr<SceneObject> moved = std::move(m_childs[srcIndex]);
		size_t index = srcIndex - 1;
		while (index >= dstIndex)
			m_childs[index + 1] = std::move(m_childs[index]);
		m_childs[dstIndex] = std::move(moved);

	} else if (srcIndex < dstIndex) {
		std::unique_ptr<SceneObject> moved = std::move(m_childs[srcIndex]);
		size_t index = srcIndex + 1;
		while (index <= dstIndex)
			m_childs[index - 1] = std::move(m_childs[index]);
		m_childs[dstIndex] = std::move(moved);
	}
}

void SceneObject::swapChild(size_t index1, size_t index2) {
	std::unique_ptr<SceneObject> temp = std::move(m_childs[index1]);
	m_childs[index1] = std::move(m_childs[index2]);
	m_childs[index2] = std::move(temp);
}

SceneObject* SceneObject::findChildWithID(ID id) const {
	auto pos = findChild_if([=](const SceneObject* obj) {
		return obj->id() == id;
	});
	
	return SAFE_UNWRAP_OBJECT_ITERATOR(pos);
}

SceneObject* SceneObject::findChildWithTag(ID tag) const {
	auto pos = findChild_if([=](const SceneObject* obj) {
		return obj->getTag() == tag;
	});

	return SAFE_UNWRAP_OBJECT_ITERATOR(pos);
}

SceneObject* SceneObject::findChildWithName(const std::string& name) const {
	auto pos = findChild_if([&](const SceneObject* obj) {
		return obj->getName() == name;
		});

	return SAFE_UNWRAP_OBJECT_ITERATOR(pos);
}


SceneObject* SceneObject::findChildWithTagRecursive(ID tag) const {
	auto result = findChild_Recursive_if([=](const SceneObject* obj) {
		return obj->getTag() == tag;
	});

	if (result.first)
		return result.second->get();

	return nullptr;
}

SceneObject* SceneObject::findChildWithNameRecursive(const std::string& name) const {
	auto result = findChild_Recursive_if([&](const SceneObject* obj) {
		return obj->getName() == name;
		});

	if (result.first)
		return result.second->get();

	return nullptr;
}

std::unique_ptr<SceneObject> SceneObject::removeChildWithID(ID id) {
	auto pos = findChild_if([=](const SceneObject* obj) {
		return obj->id() == id;
	});

	if (pos != m_childs.end()) {
		std::unique_ptr<SceneObject> obj = std::move(*pos);
		m_childs.erase(pos);
		obj->m_parent = nullptr;
		return std::move(obj);
	}
	
	return nullptr;
}

std::unique_ptr<SceneObject> SceneObject::removeChildWithTag(ID tag) {
	auto pos = findChild_if([=](const SceneObject* obj) {
		return obj->getTag() == tag;
		});

	if (pos != m_childs.end()) {
		std::unique_ptr<SceneObject> obj = std::move(*pos);
		m_childs.erase(pos);
		obj->m_parent = nullptr;
		return std::move(obj);
	}

	return nullptr;
}

std::unique_ptr<SceneObject> SceneObject::removeChildWithName(const std::string& name) {
	auto pos = findChild_if([&](const SceneObject* obj) {
		return obj->getName() == name;
		});

	if (pos != m_childs.end()) {
		std::unique_ptr<SceneObject> obj = std::move(*pos);
		m_childs.erase(pos);
		obj->m_parent = nullptr;
		return std::move(obj);
	}

	return nullptr;
}

std::unique_ptr<SceneObject> SceneObject::removeChild(SceneObject* c) {
	auto pos = findChild_if([=](const SceneObject* obj) {
		return obj == c;
		});

	if (pos != m_childs.end()) {
		std::unique_ptr<SceneObject> obj = std::move(*pos);
		m_childs.erase(pos);
		obj->m_parent = nullptr;
		return std::move(obj);
	}

	return nullptr;
}


void SceneObject::removeFromParent() {
	if (!m_parent)
		return;
	m_parent->removeChildWithID(m_id);
}


void SceneObject::removeAllChildren() {
	m_childs.clear();
}


size_t SceneObject::childCountRecursive() const {
	size_t cnt = 0;
	depthFirstTraverse([&](SceneObject* obj, bool& stop) {
		cnt += obj->childCount();
		return true;
	});
	return cnt;
}

void SceneObject::depthFirstTraverse(std::function<bool(SceneObject*, bool&)> op) const {
	std::stack<SceneObject*> traverseStack;
	bool stop = false;
	traverseStack.push(const_cast<SceneObject*>(this));
	while (!traverseStack.empty()) {
		if (stop)
			break;
		auto obj = traverseStack.top();
		traverseStack.pop();
		if (op(obj, stop)) {
			for (size_t i = obj->childCount(); i > 0; i--) {
				traverseStack.push(obj->childAt(i - 1));
			}
		}
	}
}

void SceneObject::breadthFirstTraverse(std::function<bool(SceneObject*, bool&)> op) const {
	std::queue<SceneObject*> traverseQueue;
	bool stop = false;
	traverseQueue.push(const_cast<SceneObject*>(this));
	while (!traverseQueue.empty()) {
		if (stop)
			break;
		auto obj = traverseQueue.front();
		traverseQueue.pop();
		if (op(obj, stop)) {
			for (size_t i = 0; i < obj->childCount(); i++) {
				traverseQueue.push(obj->childAt(i));
			}
		}
	}
}


SceneObject::ObjectVector::iterator SceneObject::findChild_if(std::function<bool(const SceneObject*)> pred) {
	for (auto c = m_childs.begin(); c != m_childs.end(); ++c) {
		if (pred((*c).get()))
			return c;
	}
	return m_childs.end();
}

SceneObject::ObjectVector::const_iterator SceneObject::findChild_if(std::function<bool(const SceneObject*)> pred) const {
	for (auto c = m_childs.cbegin(); c != m_childs.cend(); ++c) {
		if (pred((*c).get()))
			return c;
	}
	return m_childs.cend();
}

std::pair<bool, SceneObject::ObjectVector::iterator> SceneObject::findChild_Recursive_if(std::function<bool(const SceneObject*)> pred) {
	for (auto c = m_childs.begin(); c != m_childs.end(); c++) {
		if (pred((*c).get()))
			return std::make_pair(true, c);
	}

	for (auto c = m_childs.begin(); c != m_childs.end(); c++) {
		auto result = (*c)->findChild_Recursive_if(pred);
		if (result.first)
			return std::make_pair(true, result.second);
	}

	return std::make_pair(false, m_childs.end());
}

std::pair<bool, SceneObject::ObjectVector::const_iterator> SceneObject::findChild_Recursive_if(std::function<bool(const SceneObject*)> pred) const {
	for (auto c = m_childs.cbegin(); c != m_childs.cend(); c++) {
		if (pred((*c).get()))
			return std::make_pair(true, c);
	}

	for (auto c = m_childs.cbegin(); c != m_childs.cend(); c++) {
		auto result = (*c)->findChild_Recursive_if(pred);
		if (result.first)
			return std::make_pair(true, result.second);
	}

	return std::make_pair(false, m_childs.cend());
}
