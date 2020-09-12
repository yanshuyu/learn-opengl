#include"Notification.h"
#include"Util.h"

Notification::Notification(const std::string& name) : m_name(name) {
}


const std::string WindowResizedNotification::s_name = "WindowResizedNotification";

WindowResizedNotification::WindowResizedNotification(int w, int h) : Notification(WindowResizedNotification::s_name)
, m_width(w)
, m_height(h) {

}


const std::string CursorPositionChangedNotification::s_name = "CursorPositionChangedNotification";

CursorPositionChangedNotification::CursorPositionChangedNotification(float x, float y) : Notification(CursorPositionChangedNotification::s_name)
, m_x(x)
, m_y(y) {

}


const std::string MouseScrollNotification::s_name = "MouseScrollNotification";

MouseScrollNotification::MouseScrollNotification(float xoffset, float yoffSet) : Notification(MouseScrollNotification::s_name)
, m_xOffset(xoffset)
, m_yOffset(yoffSet) {

}