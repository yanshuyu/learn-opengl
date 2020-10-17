#include"SkeletonAnimationApp.h"
#include"LightControlGuiWindow.h"
#include<stdarg.h>
#include<glm/gtx/transform.hpp>


ScalarFrame makeScalarFrame(float time, float val);
ScalarFrame makeScalarFrame(float time, float in, float val, float out);
ScalarTrack* makeScalarTrack(InterpolationType interp, int numFrames, ...);


SkeletonAnimationApp::SkeletonAnimationApp(const std::string& t, int w, int h) :GLApplication(t, w, h)
, m_scene(nullptr)
, m_renderer(nullptr) {

}

bool SkeletonAnimationApp::initailize() {
	if (!__super::initailize())
		return false;

	RenderingSettings_t renderSettins;
	renderSettins.renderSize = { m_wndWidth, m_wndHeight };
	renderSettins.shadowMapResolution = { 1024.f, 1024.f };
	m_scene = std::make_unique<Scene>(glm::vec2(m_wndWidth, m_wndHeight), "deferred_rendering_scene");
	m_renderer = std::unique_ptr<Renderer>(new Renderer(renderSettins));

	m_renderer->setRenderMode(Renderer::Mode::Forward);
	ASSERT(m_renderer->isValid());

	auto shaderMgr = ShaderProgramManager::getInstance();
	auto meshMgr = MeshManager::getInstance();
	auto matMgr = MaterialManager::getInstance();
	auto texMgr = TextureManager::getInstance();
	
	auto monsterModel = meshMgr->addMesh(meshMgr->getResourceAbsolutePath() + "Alien_Animal.fbx");
	auto planeMat = matMgr->addMaterial("PlaneMaterial");
	m_animModel = monsterModel.get();


	auto cubeTexture = texMgr->addTexture(texMgr->getResourceAbsolutePath() + "wall.jpg");
	auto cubeMat = matMgr->addMaterial("CubeMaterial");
	cubeMat->m_diffuseMap = cubeTexture;
	cubeMat->m_shininess = 0.1f;
	auto cube = m_scene->addCube(cubeMat);
	cube->m_transform.setScale({ 4.f, 4.f, 4.f });
	cube->m_transform.setPosition({ -40.f, 0.f, 10.f });

	m_scene->addPlane(250, 250, planeMat);
	planeMat->m_shininess = 0.1f;

	auto obj = m_scene->addModel(monsterModel, "Monster");
	obj->m_transform.setScale({ 0.01f, 0.01f, 0.01f });
	obj->m_transform.setPosition({ 0.f, 0.f, 0.f });

	auto camera = m_scene->addCamera({ 0.f, 4.f, 16.f });
	camera->addComponent(ArcballCameraController::create());
	//camera->addComponent(FirstPersonCameraController::create());
	auto cameraController = camera->getComponent<ArcballCameraController>();
	cameraController->setPosition({ 0.f, 4.f, 16.f });


	auto dirLight = m_scene->addDirectionalLight({ 0.9f, 0.9f, 0.9f }, 0.9f, ShadowType::SoftShadow);
	dirLight->m_transform.setRotation({ -30.f , -60.f, 0.f });

	
	GuiManager::getInstance()->addWindow(new LightControlGuiWindow("Light Setting", this));

	m_scalarTrack.reset(makeScalarTrack(InterpolationType::Linear, 3, makeScalarFrame(1, 50), makeScalarFrame(4, 150), makeScalarFrame(8, 100)));
	m_scalarTrackCubic.reset(makeScalarTrack(InterpolationType::Cubic, 5,
		makeScalarFrame(0.25f, 0, 0, 0),
		makeScalarFrame(0.3833333f, -10.11282f, 0.5499259f, -10.11282f),
		makeScalarFrame(0.5f, 25.82528f, 1, 25.82528f),
		makeScalarFrame(0.6333333f, 7.925411f, 0.4500741f, 7.925411f),
		makeScalarFrame(0.75f, 0, 0, 0)));

}


void SkeletonAnimationApp::update(double dt) {
	if (!m_scene->isInitialize()) {
		ASSERT(m_scene->initialize());
		ASSERT(GuiManager::getInstance()->initialize());
	}

	m_scene->update(dt);
}


void SkeletonAnimationApp::render() {
	m_renderer->renderScene(m_scene.get());
}


void SkeletonAnimationApp::debugDraw(double dt) {
	static glm::mat4 vp;
	static glm::mat4 model(1.f);
	static glm::mat4 mvp(1.f);

	/*
	vp = glm::ortho(0.f, float(m_wndWidth), 0.f, float(m_wndHeight), -1.f, 10.f);
	m_scalarTrack->setInterpolationType(InterpolationType::Linear);
	DebugDrawer::drawScalarTrack(m_scalarTrack.get(), 1.5, 100, LoopType::NoLoop, 50, 10, 4, { 0, 1, 0 }, vp);
	DebugDrawer::drawScalarTrack(m_scalarTrack.get(), 1.5, 100, LoopType::Loop, 50, 150, 4, { 0, 1, 0 }, vp);
	DebugDrawer::drawScalarTrack(m_scalarTrack.get(), 1.5, 100, LoopType::PingPong, 50, 300, 4, { 0, 1, 0 }, vp);

	m_scalarTrack->setInterpolationType(InterpolationType::Constant);
	DebugDrawer::drawScalarTrack(m_scalarTrack.get(), 1.5, 100, LoopType::NoLoop, 50, 450, 4, { 0, 1, 1 }, vp);
	DebugDrawer::drawScalarTrack(m_scalarTrack.get(), 1.5, 100, LoopType::Loop, 50, 600, 4, { 0, 1, 1 }, vp);
	DebugDrawer::drawScalarTrack(m_scalarTrack.get(), 1.5, 100, LoopType::PingPong, 50, 750, 4, { 0, 1, 1 }, vp);


	model = glm::mat4(1.f);
	mvp = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(-100.f, -750.f, 0.f));
	model = glm::scale(model, glm::vec3(1.f, 80.f, 1.f));
	mvp	= vp * model;
	DebugDrawer::drawScalarTrack(m_scalarTrackCubic.get(), 2.5, 100, LoopType::NoLoop, 900, 10, 4, { 1, 0, 0 }, mvp);

	model = glm::mat4(1.f);
	mvp = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(-100.f, -600.f, 0.f));
	model = glm::scale(model, glm::vec3(1.f, 80.f, 1.f));
	mvp = vp * model;
	DebugDrawer::drawScalarTrack(m_scalarTrackCubic.get(), 2.5, 100, LoopType::Loop, 900, 10, 4, { 1, 0, 0 }, mvp);

	model = glm::mat4(1.f);
	mvp = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(-100.f, -450.f, 0.f));
	model = glm::scale(model, glm::vec3(1.f, 80.f, 1.f));
	mvp = vp * model;
	DebugDrawer::drawScalarTrack(m_scalarTrackCubic.get(), 2.5, 100, LoopType::PingPong, 900, 10, 4, { 1, 0, 0 }, mvp);
	*/

	if (m_animModel->hasSkeleton()) {
		model = glm::mat4(1.f);
		model = glm::translate(model, { -20.f, 0.f, 0.f });
		model = glm::scale(model, { 0.01f, 0.01f, 0.01f });

		vp = m_scene->getActiveCamera()->projMatrix * m_scene->getActiveCamera()->viewMatrix;
		mvp = vp * model;
		DebugDrawer::drawPose(m_animModel->getSkeleton()->getResPose(), { 0.f, 1.f, 0.f }, mvp);
		//model = glm::mat4(1.f);
		//model = glm::translate(model, { 15.f, 10.f, 0.f });
		//model = glm::scale(model, { 0.1f, 0.1f, 0.1f });
		//mvp = vp * model;
		//DebugDrawer::drawPose(m_Model->getSkeleton()->getBindPose(), { 1.f, 0.f, 0.f }, mvp);

		static double animTime = 0;
		animTime += dt;
		model = glm::mat4(1.f);
		model = glm::translate(model, { 20.f, 0.f, 0.f });
		model = glm::scale(model, { 0.01f, 0.01f, 0.01f });
		mvp = vp * model;

		Pose animatedPose = m_animModel->getSkeleton()->getResPose();
		animatedPose = m_animModel->animationAt(0)->sample(animatedPose, animTime, LoopType::PingPong);
		DebugDrawer::drawPose(animatedPose, { 1.f, 0.f, 0.f }, mvp);
	}
}

void SkeletonAnimationApp::onWindowResized(int width, int height) {
	__super::onWindowResized(width, height);
	m_scene->onWindowReSize(width, height);
	m_renderer->onWindowResize(width, height);
}



ScalarTrack* makeScalarTrack(InterpolationType interp, int numFrames, ...) {
	ScalarTrack* track = new ScalarTrack();
	track->setInterpolationType(interp);
	track->resize(numFrames);

	va_list args;
	va_start(args, numFrames);

	for (int i = 0; i < numFrames; ++i) {
		track->operator[](i) = va_arg(args, ScalarFrame);
	}

	va_end(args);

	return track;
}


ScalarFrame makeScalarFrame(float time, float val) {
	ScalarFrame f;
	f.m_time = time;
	f.m_value = val;

	return f;
}

ScalarFrame makeScalarFrame(float time, float in, float val, float out) {
	ScalarFrame f;
	f.m_time = time;
	f.m_in = in;
	f.m_value = val;
	f.m_out = out;

	return f;
}