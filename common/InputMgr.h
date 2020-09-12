#pragma once
#include"Singleton.h"
#include<GLFW/glfw3.h>
#include<bitset>


enum class KeyCode {
	Unknown = GLFW_KEY_UNKNOWN,
	Q = GLFW_KEY_Q,
	W = GLFW_KEY_W,
	E = GLFW_KEY_E,
	R = GLFW_KEY_R,
	T = GLFW_KEY_T,
	Y = GLFW_KEY_Y,
	U = GLFW_KEY_U,
	I = GLFW_KEY_I,
	O = GLFW_KEY_O,
	P = GLFW_KEY_P,
	A = GLFW_KEY_A,
	S = GLFW_KEY_S,
	D = GLFW_KEY_D,
	F = GLFW_KEY_F,
	G = GLFW_KEY_G, 
	H = GLFW_KEY_H,
	J = GLFW_KEY_J,
	K = GLFW_KEY_K,
	L = GLFW_KEY_L,
	Z = GLFW_KEY_Z,
	X = GLFW_KEY_X,
	C = GLFW_KEY_C,
	V = GLFW_KEY_V,
	B = GLFW_KEY_B,
	N = GLFW_KEY_N,
	M = GLFW_KEY_M,

	Left = GLFW_KEY_LEFT,
	Right = GLFW_HAT_RIGHT,
	Up = GLFW_KEY_UP,
	Down = GLFW_KEY_DOWN,

	LCtrl = GLFW_KEY_LEFT_CONTROL,
	RCtrl = GLFW_KEY_RIGHT_CONTROL,
	LAlt = GLFW_KEY_LEFT_ALT,
	RAlt = GLFW_KEY_RIGHT_ALT,
	LShift = GLFW_KEY_LEFT_SHIFT,
	RShift = GLFW_KEY_RIGHT_SHIFT,

	Space = GLFW_KEY_SPACE,
	ESC = GLFW_KEY_ESCAPE,
	Tab = GLFW_KEY_TAB,
	Comma = GLFW_KEY_COMMA,
	Period = GLFW_KEY_PERIOD,
	LBracket = GLFW_KEY_LEFT_BRACKET,
	RBracket = GLFW_KEY_RIGHT_BRACKET,

	F1 = GLFW_KEY_F1,
	F2 = GLFW_KEY_F2,
	F3 = GLFW_KEY_F3,
	F4 = GLFW_KEY_F4,
	F5 = GLFW_KEY_F5,
	F6 = GLFW_KEY_F6,
	F7 = GLFW_KEY_F7,
	F8 = GLFW_KEY_F8,
	F9 = GLFW_KEY_F9,
	F10 = GLFW_KEY_F10,
	F11 = GLFW_KEY_F11,
	F12 = GLFW_KEY_F12,

	Last = GLFW_KEY_LAST,
};


enum class MouseButtonCode {
	Left = GLFW_MOUSE_BUTTON_LEFT,
	Right = GLFW_MOUSE_BUTTON_RIGHT,
	Middle = GLFW_MOUSE_BUTTON_MIDDLE,
	Last,
};


class InputManager : public Singleton<InputManager> {
public:
	bool isKeyPressed(KeyCode k) const;
	bool isKeyUp(KeyCode k) const;
	bool isKeyDown(KeyCode k) const;

	bool isMouseButtonPressed(MouseButtonCode b) const;
	bool isMouseButtonUp(MouseButtonCode b) const;
	bool isMouseButtonDown(MouseButtonCode b) const;

	std::pair<float, float> getCursorPosition() const;

	void update();

	void setWindow(GLFWwindow* wnd);

	inline GLFWwindow* getWindow() const {
		return m_wnd;
	}
private:
	bool isPrintableKey(int k);
	bool isFunctionalKey(int k);

private:
	GLFWwindow* m_wnd;
	std::bitset<size_t(KeyCode::Last)> m_lastFrameKeyMask;
	std::bitset<size_t(KeyCode::Last)> m_thisFrameKeyMask;
	std::bitset<3> m_lastFrameMouseButtonMask;
	std::bitset<3> m_thisFrameMouseButtonMask;
};