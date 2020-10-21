#pragma once
#include<stdexcept>

class AppException: public std::runtime_error {
public:
	enum class Error {
		GLFW_ERROR,
		GLFW_INIT_FAILED,
		GLFW_CREATE_WND_FAILED,
		GLAD_LOAD_GL_FAILED,

		ANIMATOR_CONDCMP_NO_COMPARER,
		ANIMATOR_CONCMP_BAD_CONVAR,
	};

public:
	AppException(Error e, const std::string& desc);
	virtual ~AppException() {}
	Error error() const;
protected:
	Error m_error;
};