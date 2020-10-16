#include"GLApplication.h"
#include"NotificationCenter.h"
#include<iamgui/imgui.h>
#include<iamgui/imgui_impl_glfw.h>
#include<iamgui/imgui_impl_opengl3.h>
#include"DebugDrawer.h"


GLApplication::GLApplication(const std::string& wndTitle, size_t wndWidth, size_t wndHeight, size_t major, size_t minor):m_wndName(wndTitle)
, m_wndWidth(wndWidth)
, m_wndHeight(wndHeight)
, m_initailized(false)
, m_glfwWnd(nullptr)
, m_glMajorVersion(major)
, m_glMinorVersion(minor){

}


bool GLApplication::initailize() {
	glfwSetErrorCallback([](int error, const char* desc) {
		throw AppException(AppException::Error::GLFW_ERROR, desc);
	});

	if (!glfwInit())
		throw AppException(AppException::Error::GLFW_INIT_FAILED, "GLFW init failed!");

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, m_glMajorVersion);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, m_glMinorVersion);
	
	// create window and associate opengl context
	m_glfwWnd = glfwCreateWindow(m_wndWidth, m_wndHeight, m_wndName.c_str(), nullptr, nullptr);
	if (m_glfwWnd == nullptr) {
		shutdown();
		throw AppException(AppException::Error::GLFW_CREATE_WND_FAILED, "GLFW create window failed!");
	}

	// set current opengl context
	glfwMakeContextCurrent(m_glfwWnd);

	// gl function loader
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		shutdown();
		throw AppException(AppException::Error::GLAD_LOAD_GL_FAILED, "GLAD load gl failed!");
	}

	glfwSwapInterval(1);

	glfwSetFramebufferSizeCallback(m_glfwWnd, [](GLFWwindow* wnd, int width, int height) {
		ASSERT(glfwGetWindowUserPointer(wnd));
		GLApplication* app = static_cast<GLApplication*>(glfwGetWindowUserPointer(wnd));
		app->injectWindowSize(width, height);
		WindowResizedNotification nc(width, height);
		NotificationCenter::getInstance()->postNotification(&nc);
	});
		
	glfwSetWindowUserPointer(m_glfwWnd, this);
	
	InputManager::getInstance()->setWindow(m_glfwWnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); //(void)io;
	io.Fonts->AddFontDefault();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	const char* glsl_version = "#version 130";
	ImGui_ImplGlfw_InitForOpenGL(m_glfwWnd, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

#ifdef _DEBUG
	DebugDrawer::setup();
#endif // _DEBUG


	m_initailized = true;

	return m_initailized;
}

void GLApplication::run() {
	if (!m_initailized) {
		_ASSERT_EXPR(initailize(), "App init failed!");
	}

	double now = glfwGetTime();
	double last = now;
	while (!glfwWindowShouldClose(m_glfwWnd)) {
		glfwPollEvents();

		InputManager::getInstance()->update();
		update(now - last);
		
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		render();
		
#ifdef _DEBUG
		debugDraw(now - last);
#endif // _DEBUG

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(m_glfwWnd);

		last = now;
		now = glfwGetTime();
	}

	shutdown();
}


void GLApplication::onWindowResized(int width, int height) {
	//GLCALL(glViewport(0, 0, width, height));
	m_wndWidth = width;
	m_wndHeight = height;
}

void GLApplication::shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

#ifdef _DEBUG
	DebugDrawer::clenup();
#endif // _DEBUG


	if (m_glfwWnd != nullptr) {
		glfwDestroyWindow(m_glfwWnd);
		glfwTerminate();
		m_glfwWnd = nullptr;
	}
}
