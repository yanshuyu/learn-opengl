#pragma once
#include<string>


class Notification {
protected:
	Notification(const std::string& name);

public:
	virtual ~Notification() {}
	const std::string m_name;
};


class WindowResizedNotification : public Notification {
public:
	WindowResizedNotification(int w, int h);
	
public:
	static const std::string s_name;
	const int m_width;
	const int m_height;
};



class CursorPositionChangedNotification : public Notification {
public:
	CursorPositionChangedNotification(float x, float y);

public:
	static const std::string s_name;
	const float m_x;
	const float m_y;
};



class MouseScrollNotification : public Notification {
public:
	MouseScrollNotification(float xoffset, float yoffSet);

public:
	static const std::string s_name;
	const float m_xOffset;
	const float m_yOffset;
};