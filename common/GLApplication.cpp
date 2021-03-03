#include"GLApplication.h"
#include"NotificationCenter.h"
#include<iamgui/imgui.h>
#include<iamgui/imgui_impl_glfw.h>
#include<iamgui/imgui_impl_opengl3.h>
#include"DebugDrawer.h"
#include"FileSystem.h"

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
#ifdef _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#endif // _DEBUG

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
		app->onWindowResized(width, height);
		WindowResizedNotification nc(width, height);
		NotificationCenter::getInstance()->postNotification(&nc);
	});

	GLint ctxFlags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &ctxFlags);
	if (ctxFlags & GL_CONTEXT_FLAG_DEBUG_BIT) { // opengl context is created with debug output is enabled
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback([](GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* msg, const void* userData) {
			 GLApplication* app = const_cast<GLApplication*>(static_cast<const GLApplication*>(userData));
			 app->onOpenglDebugError(source, type, id, severity, length, msg);
			}, this);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PERFORMANCE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
	}
		
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

	fs::path lanchPath = FileSystem::Default.currentWorkingDirectory();
	FileSystem::Default.setCurrentWorkingDirectory(lanchPath.parent_path());
	FileSystem::Default.setHomeDirectory(FileSystem::Default.currentWorkingDirectory());

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
		GuiManager::getInstance()->render();

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


void GLApplication::onOpenglDebugError(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* msg) {
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
		return;

	std::cout << "---------------" << std::endl;
	std::cout << "OpenGL Debug message (" << id << "): " << msg << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	}
	std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	}
	std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} 
	std::cout << std::endl;
	std::cout << "---------------" << std::endl;
}
