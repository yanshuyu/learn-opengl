#pragma once
#include"SceneObject.h"
#include"RendererCore.h"
#include"MeshLoader.h"
#include<functional>



class Notification;
class LightComponent;
class CameraComponent;
class Renderer;



class Scene {
	typedef std::vector<std::unique_ptr<SceneObject>> ObjectVector;
public:
	enum Tag {
		Defalut,
		Camera,
		DirectionalLight,
		PointLight,
		SpotLight,
	};

public:
	Scene(const std::string& name = "");
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
	
	SceneObject* addModel( const std::string& file, int loadOptions = MeshLoader::Option::LoadMaterials | MeshLoader::Option::LoadAnimations,const std::string& name = "");
	SceneObject* addGrid(float w, float d, float spacing, std::weak_ptr<IMaterial> mat = std::weak_ptr<IMaterial>());
	SceneObject* addPlane(float w, float d, std::weak_ptr<IMaterial> mat = std::weak_ptr<IMaterial>());
	SceneObject* addCube(std::weak_ptr<IMaterial> mat = std::weak_ptr<IMaterial>());
	SceneObject* addCamera(const glm::vec3& pos = glm::vec3(0.f), const glm::vec3& rot = glm::vec3(0.f), const glm::vec3& bgColor = glm::vec3(0.f));

	SceneObject* addDirectionalLight(const glm::vec3& color, float intensity = 1.f, ShadowType shadowType = ShadowType::HardShadow);
	SceneObject* addPointLight(const glm::vec3& color, float range = 50, float intensity = 1.f, ShadowType shadowType = ShadowType::NoShadow);
	SceneObject* addSpotLight(const glm::vec3& color, float innerAngle = 30.f, float outterAngle = 60.f, float range = 50.f,float intesity = 1.f, ShadowType shadowType = ShadowType::NoShadow,float shadowStrength = 0.5f);

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
	inline void setRenderer(Renderer* renderer) {
		m_renderContext.setRenderer(renderer);
		m_renderContext.clearMatrix();
	}

	inline Renderer* getRenderer() const {
		return m_renderContext.getRenderer();
	}

	void setEnviromentLight(const glm::vec3& sky, const glm::vec3& ground);

	inline std::pair<glm::vec3, glm::vec3> getEnviromentLight() const {
		return std::pair<glm::vec3, glm::vec3>(m_ambientSky, m_ambientGround);
	}

	glm::vec2 getRenderSize() const;

	void render();

	//
	// events
	//
	void onWindowResize(float width, float height);
	void onComponentAttach(SceneObject* obj, Component* c) {};
	void onComponentDetach(SceneObject* obj, Component* c) {};
	void onCameraAdded(SceneObject* obj, CameraComponent* camera);
	void onCameraRemoved(SceneObject* obj, CameraComponent* camera);
	void onLightAdded(SceneObject* obj, LightComponent* light);
	void onLightRemoved(SceneObject* obj, LightComponent* light);

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

	inline size_t cameraCount() const {
		return m_cameras.size();
	}

	inline CameraComponent* cameraAt(size_t idx) {
		return m_cameras[idx];
	}

	inline size_t lightCount() const {
		return m_lights.size();
	}

	LightComponent* lightAt(size_t idx) {
		return m_lights[idx];
	}


private:
	ID m_id;
	std::string m_name;
	bool m_isInitialize;

	std::unique_ptr<SceneObject> m_rootObject;

	std::vector<LightComponent*> m_lights;
	std::vector<CameraComponent*> m_cameras;
	CameraComponent* m_mainCamera;
	
	RenderContext m_renderContext;

	glm::vec3 m_ambientGround;
	glm::vec3 m_ambientSky;
};