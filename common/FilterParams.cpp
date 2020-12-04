#include"FilterParams.h"


void FilterParamGroup::addParam(FilterParam* p) {
	for (auto& _p : m_params) {
		if (_p.get() == p)
			return;
	}

	m_params.push_back(std::unique_ptr<FilterParam>(p));
}


FilterParam* FilterParamGroup::getParam(const std::string& name) const {
	for (auto& p : m_params) {
		if (p->m_name == name)
			return p.get();
	}

	return nullptr;
}


bool FilterParamGroup::removeParam(const std::string& name) {
	auto pos = m_params.end();
	for (auto p = m_params.begin(); p != m_params.end(); p++) {
		if ((*p)->m_name == name) {
			pos = p;
			break;
		}
	}

	if (pos != m_params.end()) {
		m_params.erase(pos);
		return true;
	}

	return false;
}

template<>
TFilterParam<int>::TFilterParam(const std::string& name, const int& val) : FilterParam(name, Type::Int)
, m_value(val) {

}

template<>
TFilterParam<float>::TFilterParam(const std::string& name, const float& val) : FilterParam(name, Type::Float)
, m_value(val) {

}

template<>
TFilterParam<bool>::TFilterParam(const std::string& name, const bool& val) : FilterParam(name, Type::Bool)
, m_value(val) {

}


template TFilterParam<int>;
template TFilterParam<float>;
template TFilterParam<bool>;