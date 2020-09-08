#pragma once
#include"SceneObject.h"
#include"Renderer.h"
#include<functional>

class Scene {
	typedef std::vector<std::unique_ptr<SceneObject>> ObjectVector;
public:
	Scene(const std::string& name = "", Renderer* renderer = nullptr);
	virtual ~Scene() {}

	Scene(const Scene& other) = delete;
	Scene& operator = (const Scene& other) = delete;

	//
	// life cycle
	//
	virtual bool initialize();
	void update(double dt);

	//
	// scene object managment
	//
	SceneObject* addObject(const std::string& name = "");
	void addOject(SceneObject* object);
	void addObject(std::unique_ptr<SceneObject>&& object);
	
	SceneObject* findObjectWithID(ID id) const;
	SceneObject* findObjectWithTag(ID tag) const;
	SceneObject* findObjectWithName(const std::string& name) const;
	SceneObject* findObjectWithTagRecursive(ID tag) const;
	SceneObject* findObjectWithNameRecursive(const std::string& name) const;
	
	std::unique_ptr<SceneObject> removeObjectWithID(ID id);
	std::unique_ptr<SceneObject> removeObjectWithTag(ID tag);
	std::unique_ptr<SceneObject> removeObjectWithName(const std::string& name);

	void clearObjects();
	size_t objectCount() const;
	size_t objectCountRecursive() const;

	//
	// traverse
	//
	void depthFirstVisit(std::function<bool(SceneObject*, bool&)> op) const;
	void breathFirstVisit(std::function<bool(SceneObject*, bool&)> op) const;

	//
	// rendering
	//
	void render();

	inline void setRenderer(Renderer* renderer) {
		m_renderer = renderer;
	}

	inline Renderer* getRenderer() const {
		return m_renderer;
	}


	//
	// public getter setter
	//
	inline bool isInitialize() const {
		return m_isInitialize;
	}

	inline void setName(const std::string& name) {
		m_name = name;
	}

	inline std::string getName() const {
		return m_name;
	}

	inline ID id() const {
		return m_id;
	}


protected:
	bool m_isInitialize;
	std::string m_name;
	ID m_id;
	std::unique_ptr<SceneObject> m_rootObject;
	Renderer* m_renderer;
};