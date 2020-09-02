#include"GLApplication.h"


GLApplication::GLApplication(const std::string& wndTitle, size_t wndWidth, size_t wndHeight, size_t major, size_t minor):m_wndName(wndTitle)
, m_wndWidth(wndWidth)
, m_wndHeight(wndHeight)
, m_initailized(false)
, m_glfwWnd(nullptr)
, m_glMajorVersion(major)
, m_glMinorVersion(minor){

}


bool GLApplication::initailize() {
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
	});
	
	GLCALL(glViewport(0, 0, m_wndWidth, m_wndHeight));

	glfwSetWindowUserPointer(m_glfwWnd, this);

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
		update( now - last);
		render();
		glfwSwapBuffers(m_glfwWnd);
		glfwPollEvents();

		last = now;
		now = glfwGetTime();
	}

	shutdown();
}


void GLApplication::onWindowResized(int width, int height) {
	GLCALL(glViewport(0, 0, width, height));
	m_wndWidth = width;
	m_wndHeight = height;
}

void GLApplication::shutdown() {
	if (m_glfwWnd != nullptr) {
		glfwTerminate();
		m_glfwWnd = nullptr;
	}
}
