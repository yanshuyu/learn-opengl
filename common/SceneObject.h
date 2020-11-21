#pragma once
#include"Component.h"
#include"TransformComponent.h"
#include"MeshRenderComponent.h"
#include"RendererCore.h"
#include<vector>
#include<memory>
#include<functional>

class Renderer;
class Scene;

enum class Layer {
	Default,
	Transparency,
};

class SceneObject {
	friend class Scene;
	typedef std::vector<std::shared_ptr<Component>> ComponentVector;
	typedef std::vector<std::unique_ptr<SceneObject>> ObjectVector;

public:
	SceneObject(const std::string& name = "");
	virtual ~SceneObject();

	SceneObject(const SceneObject& other) = delete;
	SceneObject(SceneObject&& rv) = delete;
	SceneObject& operator = (const SceneObject& other) = delete;
	SceneObject& operator = (SceneObject&& rv) = delete;

	SceneObject* copy() const;
	
	//
	// life cycle
	//
	bool initialize();
	void update(double dt);
	void render(RenderContext* context) const;
	
	//
	// component managment
	//
	bool addComponent(Component* c);
	bool addComponent(std::shared_ptr<Component> c);
	template<typename T, typename... Args> T* addComponent(Args&& ... args) {
		for (auto& c : m_components) {
			if (c->typeId() == T::s_typeId)
				return static_cast<T*>(c.get());
		}

		auto c = new T(std::forward<Args>(args)...);
		addComponent(c);

		return c;
	}

	bool hasComponent(const ID id) const;
	template<typename T> bool hasComponent() const {
		return hasComponent(T::s_typeId);
	}

	std::weak_ptr<Component> getComponent(const ID id) const;
	template<typename T> std::weak_ptr<T> getComponent() {
		auto weak_c = getComponent(T::s_typeId);
		if (weak_c.expired())
			return std::weak_ptr<T>();

		return std::static_pointer_cast<T>(weak_c.lock());
	}

	bool removeComponent(const ID id);
	template<typename T> bool removeComponent() {
		return removeComponent(T::s_typeId);
	}

	void removeAllComponent();

	inline size_t componentCount() const {
		return m_components.size();
	}

	inline Component* componentAt(size_t index) const {
		return m_components[index].get();
	}

	//
	// object hiearcy managment
	//
	void addChild(SceneObject* c);
	void addChild(std::unique_ptr<SceneObject>&& c);
	void insertChild(SceneObject* c, size_t index);
	void insertChild(std::unique_ptr<SceneObject>&& c, size_t index);
	void movechild(size_t srcIndex, size_t dstIndex);
	void swapChild(size_t index1, size_t index2);
	
	SceneObject* findChildWithID(ID id) const;
	SceneObject* findChildWithTag(ID tag) const;
	SceneObject* findChildWithName(const std::string& name) const;
	SceneObject* findChildWithTagRecursive(ID tag) const;
	SceneObject* findChildWithNameRecursive(const std::string& name) const;

	std::unique_ptr<SceneObject> removeChildWithID(ID id);
	std::unique_ptr<SceneObject> removeChildWithTag(ID tag);
	std::unique_ptr<SceneObject> removeChildWithName(const std::string& name);
	std::unique_ptr<SceneObject> removeChild(SceneObject* c);
	
	void removeAllChildren();
	void removeFromParent();

	inline SceneObject* getParent() const {
		return m_parent;
	}

	inline size_t childCount() const {
		return m_childs.size();
	}

	size_t childCountRecursive() const;

	inline SceneObject* childAt(size_t index) const {
		return m_childs[index].get();
	}


	//
	// object traverse
	//
	void depthFirstTraverse(std::function<bool(SceneObject*, bool&)> op) const;
	void breadthFirstTraverse(std::function<bool(SceneObject*, bool&)> op) const;

	//
	// public getter setter
	//
	inline ID id() const {
		return m_id;
	}

	inline void setName(const std::string& name) {
		m_name = name;
	}

	inline std::string getName() const {
		return m_name;
	}

	inline void setTag(ID tag) {
		m_tag = tag;
	}

	inline ID getTag() const {
		return m_tag;
	}

	inline Layer getLayer() const {
		return m_layer;
	}

	inline void setLayer(Layer layer) {
		m_layer = layer;
	}


	inline Scene* getParentScene() const {
		return m_parentScene;
	}

private:	
	ObjectVector::iterator findChild_if(std::function<bool(const SceneObject*)> pred);
	ObjectVector::const_iterator findChild_if(std::function<bool(const SceneObject*)> pred) const;
	std::pair<bool, ObjectVector::iterator> findChild_Recursive_if(std::function<bool(const SceneObject*)> pred);
	std::pair<bool, ObjectVector::const_iterator> findChild_Recursive_if(std::function<bool(const SceneObject*)> pred) const;

public:
	TransformComponent  m_transform;
	bool m_isVisible;
	bool m_isEnable;

private:
	ID m_id;
	ID m_tag;
	Layer m_layer;
	std::string m_name;
	SceneObject* m_parent;
	Scene* m_parentScene;
	std::vector<std::unique_ptr<SceneObject>> m_childs;
	std::vector<std::shared_ptr<Component>> m_components; // for components have refrence to other components using a weak ptr
};
