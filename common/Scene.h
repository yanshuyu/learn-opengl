#pragma once
#include"SceneObject.h"
#include"Renderer.h"
#include<functional>


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
	
	SceneObject* addGrid(float w, float d, float spacing, std::shared_ptr<Material> mat = nullptr);
	SceneObject* addPlane(float w, float d, std::shared_ptr<Material> mat = nullptr);
	SceneObject* addCube(std::shared_ptr<Material> mat = nullptr);
	SceneObject* addCamera(const glm::vec3& p = glm::vec3(0.f), const glm::vec3& r = glm::vec3(0.f), const glm::vec3& bgColor = glm::vec3(0.f));

	SceneObject* addDirectionalLight(const glm::vec3& color, float intensity = 1.f);
	SceneObject* addPointLight(const glm::vec3& color, float range = 50, float intensity = 1.f);
	SceneObject* addSpotLight(const glm::vec3& color, 
								float innerAngle = 30.f, 
								float outterAngle = 60.f, 
								float range = 50.f,
								float intesity = 1.f, 
								ShadowType shadowType = ShadowType::NoShadow,
								float shadowStrength = 0.5f);

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
	SceneRenderInfo_t* gatherSceneRenderInfo() const;
	void render(RenderContext* context);
	
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

	inline void onWindowReSize(float w, float h) {
		m_windowSize = glm::vec2(w, h);
	}


private:
	ID m_id;
	std::string m_name;
	bool m_isInitialize;

	std::unique_ptr<SceneObject> m_rootObject;
	glm::vec2 m_windowSize;
};