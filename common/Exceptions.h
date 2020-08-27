#pragma once
#include<stdexcept>

class AppException: public std::runtime_error {
public:
	enum class Error {
		GLFW_INIT_FAILED,
		GLFW_CREATE_WND_FAILED,
		GLAD_LOAD_GL_FAILED,
	};

public:
	AppException(Error e, const std::string& desc);
	virtual ~AppException() {}
	Error error() const;
protected:
	Error m_error;
};