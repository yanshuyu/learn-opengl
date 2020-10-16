#include"Skeleton.h"

void Skeleton::set(const Pose& resPose, const Pose& bindPose, const std::vector<std::string>& jointNames) {
	m_resPose = resPose;
	m_bindPose = bindPose;
	m_jointsName = jointNames;
	calcInvBindPose();
}


int Skeleton::getJointId(const std::string& jointName) const {
	for (size_t i = 0; i < m_jointsName.size(); i++) {
		if (m_jointsName[i] == jointName)
			return i;
	}

	return -1;
}

void Skeleton::calcInvBindPose() {
	m_resolvedInvBindPose = m_bindPose.resolvePose();
	for (size_t i = 0; i < m_resolvedInvBindPose.size(); i++) {
		m_resolvedInvBindPose[i] = inverse(m_resolvedInvBindPose[i]);
	}
}