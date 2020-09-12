#include"InputMgr.h"
#include"NotificationCenter.h"


bool InputManager::isKeyPressed(KeyCode k) const {
	//return glfwGetKey(m_glfwWnd, int(k)) == GLFW_PRESS;
	return m_lastFrameKeyMask.test(size_t(k)) && m_thisFrameKeyMask.test(size_t(k));
}

bool InputManager::isKeyUp(KeyCode k) const {
	return m_lastFrameKeyMask.test(size_t(k)) && !m_thisFrameKeyMask.test(size_t(k));
}

bool InputManager::isKeyDown(KeyCode k) const {
	return !m_lastFrameKeyMask.test(size_t(k)) && m_thisFrameKeyMask.test(size_t(k));
}


bool InputManager::isMouseButtonPressed(MouseButtonCode b) const {
	return m_lastFrameMouseButtonMask.test(size_t(b)) && m_thisFrameMouseButtonMask.test(size_t(b));
}

bool InputManager::isMouseButtonUp(MouseButtonCode b) const {
	return m_lastFrameMouseButtonMask.test(size_t(b)) && !m_thisFrameMouseButtonMask.test(size_t(b));
}

bool InputManager::isMouseButtonDown(MouseButtonCode b) const {
	return !m_lastFrameMouseButtonMask.test(size_t(b)) && m_thisFrameMouseButtonMask.test(size_t(b));
}


std::pair<float, float> InputManager::getCursorPosition() const {
	double x, y;
	glfwGetCursorPos(m_wnd, &x, &y);
	return std::make_pair(x, y);
}

void InputManager::setWindow(GLFWwindow* wnd) {
	m_wnd = wnd;
	
	glfwSetCursorPosCallback(wnd, [](GLFWwindow* w, double x, double y) {
		CursorPositionChangedNotification nc(x, y);
		NotificationCenter::getInstance()->postNotification(&nc);
	});

	glfwSetScrollCallback(wnd, [](GLFWwindow* w, double xOffset, double yOffset) {
		MouseScrollNotification nc(xOffset, yOffset);
		NotificationCenter::getInstance()->postNotification(&nc);
	});
}


void InputManager::update() {
	m_lastFrameKeyMask = m_thisFrameKeyMask;
	for (size_t k = 0; k < size_t(KeyCode::Last); k++) {
		if (isPrintableKey(k) || isFunctionalKey(k)) {
			m_thisFrameKeyMask.set(k, glfwGetKey(m_wnd, k) == GLFW_PRESS);
		}
	}

	m_lastFrameMouseButtonMask = m_thisFrameMouseButtonMask;
	for (size_t b = size_t(MouseButtonCode::Left); b < size_t(MouseButtonCode::Last); b++) {
		m_thisFrameMouseButtonMask.set(size_t(b), glfwGetMouseButton(m_wnd, b) == GLFW_PRESS);
	}
}


bool InputManager::isPrintableKey(int k) {
	return k >= 32 && k <= 96;
}

bool InputManager::isFunctionalKey(int k) {
	return k >= 256 && k <= 348;
}