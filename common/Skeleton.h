#pragma once
#include"Pose.h"
#include<string>

// skeleton manage bind-pose, inverse-bind-pose that shared
// by several animation clip
class Skeleton {
public:
	Skeleton() = default;

	void set(const Pose& resPose,
		const Pose& invBindPose,
		const std::vector<std::string>& jointNames, 
		const glm::mat4& invRoot = glm::mat4(1.f));

	int getJointId(const std::string& jointName) const;
	
	inline const Pose& getResPose() const {
		return m_resPose;
	}

	inline const Pose& getInvBindPose() const {
		return m_invBindPose;
	}

	inline glm::mat4 getInvRootTransform() const {
		return m_invRootTransform;
	}

	inline std::vector<std::string> getJointNames() const {
		return m_jointsName;
	}

	inline size_t size() const {
		return m_resPose.size();
	}


protected:
	//void calcBindPoseAndInvBindPose(const Pose& worldBindPose);

protected:
	Pose m_resPose;
	//Pose m_bindPose;
	Pose m_invBindPose;
	std::vector<std::string> m_jointsName;
	glm::mat4 m_invRootTransform;
};