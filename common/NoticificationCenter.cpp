#include"NotificationCenter.h"


void NotificationCenter::addObserver(void* obj, const std::string& notificationName, const NotificationCallbak& callback) {
	auto observerMap = m_eventCenter.find(notificationName);
	if (observerMap == m_eventCenter.end()) {
		m_eventCenter.insert({ notificationName, std::unordered_map<void*, NotificationCallbak>() });
		m_eventCenter.at(notificationName).insert({ obj, callback });
		return;
	}

	observerMap->second[obj] = callback;
}

void NotificationCenter::removeObserber(void* obj, const std::string& notificationName) {
	auto observerMap = m_eventCenter.find(notificationName);
	if (observerMap != m_eventCenter.end())
		observerMap->second.erase(obj);
}

void NotificationCenter::removeObserver(void* obj) {
	for (auto observerMap = m_eventCenter.begin(); observerMap != m_eventCenter.end(); observerMap++){
		observerMap->second.erase(obj);
	}
}

void NotificationCenter::removeObservers(const std::string& notificationName) {
	m_eventCenter.erase(notificationName);
}

void NotificationCenter::clearObservers() {
	m_eventCenter.clear();
}

void NotificationCenter::postNotification(const Notification* notification) const {
	auto observerMap = m_eventCenter.find(notification->m_name);
	if (observerMap != m_eventCenter.end()) {
		for (auto observer = observerMap->second.begin(); observer != observerMap->second.end(); observer++) {
			observer->second(notification);
		}
	}
}