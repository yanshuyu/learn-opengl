#pragma once
#include"Pose.h"
#include<string>

// skeleton manage bind-pose, inverse-bind-pose that shared
// by several animation clip
class Skeleton {
public:
	Skeleton() = default;

	void set(const Pose& resPose, const Pose& bindPose, const std::vector<std::string>& jointNames);

	int getJointId(const std::string& jointName) const;
	
	inline const Pose& getResPose() const {
		return m_resPose;
	}

	inline const Pose& getBindPose() const {
		return m_bindPose;
	}

	inline const Pose& getInvBindPose() const {
		return m_resolvedInvBindPose;
	}

	inline std::vector<std::string> getJointNames() const {
		return m_jointsName;
	}

	inline size_t size() const {
		return m_resPose.size();
	}

	inline void resize(size_t sz) {
		m_resPose.resize(sz);
		m_bindPose.resize(sz);
		m_resolvedInvBindPose.resize(sz);
	}

protected:
	void calcInvBindPose();

protected:
	Pose m_resPose;
	Pose m_bindPose;
	Pose m_resolvedInvBindPose;
	std::vector<std::string> m_jointsName;
};