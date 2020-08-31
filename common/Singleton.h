#pragma once



template<typename T>
class Singleton {
public:
	virtual ~Singleton() {}

	static T* getInstance() {
		static T instance;
		return &instance;
	}
};