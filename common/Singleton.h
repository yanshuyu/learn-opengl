#pragma once

template<typename T>
class Singleton {
public:
	virtual ~Singleton() {}

	static T* getInstance() {
		static T instance;
		return &instance;
	}

protected:
	Singleton() {};
	Singleton(const Singleton& other) {}
	Singleton(Singleton&& rv) {}
	Singleton& operator = (const Singleton& other) { return *this; }
	Singleton& operator = (Singleton&& rv) { return *this; }
};