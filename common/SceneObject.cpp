#include"SceneObject.h"
#include"Renderer.h"
#include<stack>
#include<queue>

#define SAFE_UNWRAP_OBJECT_ITERATOR(itr)  itr == m_childs.end() ? nullptr : itr->get();


SceneObject::SceneObject(const std::string& name) :m_name(name)
, m_id(0)
, m_transform(this)
, m_isEnable(true)
, m_isVisible(true)
, m_parent(nullptr)
, m_tag(0) {
	m_id = reinterpret_cast<unsigned long>(this);
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
			if (!c->initialize()) {
				success = false;
				stop = true;
				return false;
			}
		}
		return true;
	});

	return success;
}

void SceneObject::update(double dt) {
	depthFirstTraverse([=](SceneObject* obj, bool& stop) {
		if (!obj->m_isEnable || !obj->m_isVisible)
			return false;
		for (size_t i = 0; i < obj->componentCount(); i++) {
			auto c = obj->componentAt(i);
			c->update(dt);
		}
		return true;
	});
}

void SceneObject::render(RenderContext* context) const {
	if (!m_isVisible)
		return;

	for (auto c = m_components.begin(); c != m_components.end(); c++)
		(*c)->render(context);

	context->pushMatrix(m_transform.getMatrix());
	for (size_t i = 0; i < m_childs.size(); i++) {
		m_childs[i]->render(context);
	}
	context->popMatrix();
}

bool SceneObject::addComponent(Component* c) {
	if (!c)
		return false;

	if (findComponent(c->identifier()))
		return false;

	c->m_owner = this;
	m_components.emplace_back(c);
	return true;
}

bool SceneObject::addComponent(std::shared_ptr<Component> c) {
	if (!c)
		return false;
	
	if (c->m_owner)
		return false;

	if (findComponent(c->identifier()))
		return false;
	
	c->m_owner = this;
	m_components.push_back(c);
	return true;
}

bool SceneObject::removeComponent(const std::string& identifier) {
	auto pos = findComponent_if(identifier);
	if (pos != m_components.end()) {
		m_components.erase(pos);
		return true;
	}

	return false;
}

Component* SceneObject::findComponent(const std::string& identifier) const {
	auto pos = findComponent_if(identifier);
	if (pos != m_components.cend())
		return (*pos).get();

	return nullptr;
}

void SceneObject::addChild(SceneObject* c) {
	m_childs.emplace_back(c);
	c->m_parent = this;
}

void SceneObject::addChild(std::unique_ptr<SceneObject>&& c) {
	auto _c = c.get();
	m_childs.push_back(std::move(c));
	_c->m_parent = this;
}

void SceneObject::insertChild(SceneObject* c, size_t index) {
	m_childs.insert(m_childs.begin() + index, std::unique_ptr<SceneObject>(c));
	c->m_parent = this;
}

void SceneObject::insertChild(std::unique_ptr<SceneObject>&& c, size_t index) {
	auto _c = c.get();
	m_childs.insert(m_childs.begin() + index, std::move(c));
	_c->m_parent = this;
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


void SceneObject::clearChilds() {
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

SceneObject::ComponentVector::iterator SceneObject::findComponent_if(const std::string& identifier) {
	auto c = m_components.begin();
	for (; c != m_components.end(); c++) {
		if ((*c)->identifier() == identifier) 
			break;
	}

	return c;
}


SceneObject::ComponentVector::const_iterator SceneObject::findComponent_if(const std::string& identifier) const {
	auto c = m_components.cbegin();
	for (; c != m_components.cend(); c++) {
		if ((*c)->identifier() == identifier)
			break;
	}

	return c;
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
