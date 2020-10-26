#pragma once
#include"Transform.h"
#include<vector>


// a pose contain hiearcy transform of a skeleton 
// in a flatten array, joint's id is it's index of the array
class Pose {
public:
	Pose() = default;
	Pose(const Pose& other);
	Pose(Pose&& rv);

	Pose& operator = (const Pose& other);
	Pose& operator = (Pose&& rv);

	Pose getGlobalPose() const;
	
	void getSkinMatrix(std::vector<glm::mat4>& skinMat) const; 

	Transform getJointTransformGlobal(int jointId) const;

	inline Transform getJointTransformLocal(int jointId) const {
		return m_joints[jointId];
	}

	inline Transform& operator [] (int jointId) {
		return m_joints[jointId];
	}

	inline const Transform& operator [] (int jointId) const {
		return m_joints[jointId];
	}

	inline int getJointParent(int jointId) const {
		return m_parents[jointId];
	}

	inline void setJointParent(int jointId, int parent) {
		m_parents[jointId] = parent;
	}

	inline size_t size() const {
		return m_joints.size();
	}

	inline void resize(size_t sz) {
		m_joints.resize(sz, Transform());
		m_parents.resize(sz, -1);
	}

protected:
	std::vector<Transform> m_joints;
	std::vector<int> m_parents;
};
