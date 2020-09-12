#pragma once
#include"Notification.h"
#include"Singleton.h"
#include<unordered_map>
#include<vector>
#include<functional>

class NotificationCenter : public Singleton<NotificationCenter> {
	typedef std::function<void(const Notification*)> NotificationCallbak;
public:
	void addObserver(void* obj, const std::string& notificationName, const NotificationCallbak& callback);
	void removeObserber(void* obj, const std::string& notificationName);
	void removeObserver(void* obj);
	void removeObservers(const std::string& notificationName);
	void clearObservers();
	void postNotification(const Notification* notification) const;

private:
	std::unordered_map<std::string, std::unordered_map<void*, NotificationCallbak>> m_eventCenter;
};