#pragma once
#include"pch.h"
#include<string>


class GLApplication {
public:
	GLApplication(const std::string& wndTitle, size_t wndWidth = 1920, size_t wndHeight = 1080, size_t major = 4, size_t minor = 5);
	virtual ~GLApplication() {}

	virtual void run();

	virtual void onWindowResized(int width, int height);
	virtual void onOpenglDebugError(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* msg);

protected:
	virtual bool initailize();
	virtual void update(double dt) {};
	virtual void render() {};
#ifdef _DEBUG
	virtual	void debugDraw(double dt) {};
#endif // _DEBUG

	virtual void shutdown();

protected:
	std::string m_wndName;
	size_t m_wndWidth;
	size_t m_wndHeight;
	bool m_initailized;
	GLFWwindow* m_glfwWnd;
	size_t m_glMajorVersion;
	rsize_t m_glMinorVersion;
};