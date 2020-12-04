#pragma once
#include<string>
#include<vector>
#include<memory>

class FilterParam {
public:
	enum class Type {
		Unknown,
		Int,
		Float,
		Bool,
	};
public:
	FilterParam() : FilterParam("", Type::Unknown) {}
	FilterParam(const std::string& name, Type type) :m_name(name), m_type(type) {}
	virtual ~FilterParam() {}

	Type m_type;
	std::string m_name;
};


template<typename T>
class TFilterParam : public FilterParam {
public:
	TFilterParam() : FilterParam(), m_value() {}
	TFilterParam(const std::string& name, const T& val = T()) : FilterParam(name, Type::Unknown), m_value(val) {}

	T m_value;
};



class FilterParamGroup {
public:
	FilterParamGroup() = default;

	inline size_t paramsCount() const {
		return m_params.size();
	}

	inline const FilterParam* paramAt(size_t idx) const {
		return m_params[idx].get();
	}

	inline  FilterParam* paramAt(size_t idx) {
		return m_params[idx].get();
	}

	void addParam(FilterParam* p);

	template<typename T> 
	TFilterParam<T>* addParam(const std::string& name, const T& val = T()) {
		for (auto& p : m_params) {
			if (p->m_name == name)
				return nullptr;
		}

		auto p = new TFilterParam<T>(name, val);
		m_params.push_back(std::unique_ptr<FilterParam>(p));
	
		return p;
	}

	template<typename T>
	TFilterParam<T>* getParam(const std::string& name) const {
		for (auto& p : m_params) {
			if (p->m_name == name)
				return static_cast<TFilterParam<T>*>(p.get());
		}
	}


	FilterParam* getParam(const std::string& name) const;


	bool removeParam(const std::string& name);


	inline const std::vector<std::unique_ptr<FilterParam>>& allParams() const {
		return m_params;
	}

protected:
	std::vector<std::unique_ptr<FilterParam>> m_params;
};



typedef TFilterParam<int> FilterParamsInt;
typedef TFilterParam<float> FilterParamsFloat;
typedef TFilterParam<bool> FilterParamsBool;
