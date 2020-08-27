#include"Exceptions.h"


AppException::AppException(Error e, const std::string& desc) :std::runtime_error(desc)
, m_error(e) {

}


AppException::Error AppException::error() const {
	return m_error;
}