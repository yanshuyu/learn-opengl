#pragma once
#include"SceneObject.h"
#include"Renderer.h"
#include<functional>

class Renderer;
class Notification;


class Scene {
	friend class Renderer;
	typedef std::vector<std::unique_ptr<SceneObject>> ObjectVector;
public:
	Scene(const glm::vec2& wndSz, const std::string& name = "");
	virtual ~Scene();

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
	
	SceneObject* addCamera(const glm::vec3& p = glm::vec3(0.f), const glm::vec3& r = glm::vec3(0.f));
	CameraComponent* getCamera() const;


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
	void preRender(Renderer* renderer);
	void render(RenderContext* context);
	void afterRender(Renderer* renderer);
	
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

	inline void setWindowSize(float w, float h) {
		m_windowSize = glm::vec2(w, h);
	}

protected:
	ID m_id;
	std::string m_name;
	bool m_isInitialize;

	std::unique_ptr<SceneObject> m_rootObject;
	glm::vec2 m_windowSize;
};