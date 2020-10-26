#include"Skeleton.h"

void Skeleton::set(const Pose& resPose, const Pose& invBindPose, const std::vector<std::string>& jointNames, const glm::mat4& invRoot) {
	m_resPose = resPose;
	m_invBindPose = invBindPose;
	m_jointsName = jointNames;
	m_invRootTransform = invRoot;
}


int Skeleton::getJointId(const std::string& jointName) const {
	for (size_t i = 0; i < m_jointsName.size(); i++) {
		if (m_jointsName[i] == jointName)
			return i;
	}

	return -1;
}
