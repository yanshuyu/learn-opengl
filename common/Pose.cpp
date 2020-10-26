#include"Pose.h"

Pose::Pose(const Pose& other) {
	*this = other;
}

Pose::Pose(Pose&& rv) {
	*this = std::move(rv);
}

Pose& Pose::operator = (const Pose& other) {
	if (&other == this)
		return *this;

	resize(other.size());
	std::memcpy(m_joints.data(), other.m_joints.data(), other.size() * sizeof(Transform));
	std::memcpy(m_parents.data(), other.m_parents.data(), other.size() * sizeof(int));

	return *this;
}


Pose& Pose::operator = (Pose&& rv) {
	std::swap(m_joints, rv.m_joints);
	std::swap(m_parents, rv.m_parents);
	
	return *this;
}


Transform Pose::getJointTransformGlobal(int jointId) const {
	if (jointId < 0 || jointId >= m_joints.size())
		return Transform();

	Transform result = m_joints[jointId];
	int parentId = m_parents[jointId];
	while (parentId >= 0)	{
		result = combine(m_joints[parentId], result);
		parentId = m_parents[parentId];
	}

	return result;
}


Pose Pose::getGlobalPose() const {
	Pose result;
	result.resize(m_joints.size());
	
	for (size_t i = 0; i < m_joints.size(); i++) {
		result[i] = getJointTransformGlobal(i);
		result.m_parents[i] = m_parents[i];
	}
	
	return result;
}


void Pose::getSkinMatrix(std::vector<glm::mat4>& skinMat) const {
	if (skinMat.size() != size())
		skinMat.resize(size());

	int i = 0;
	for (; i < size(); i++) {
		int parentId = m_parents[i];
		if (parentId > i)
			break;

		glm::mat4 m = transform2Mat(m_joints[i]);
		if (parentId >= 0)
			m = skinMat[parentId] * m;
		
		skinMat[i] = m;
	}

	for (; i < size(); i++) {
		skinMat[i] = transform2Mat(getJointTransformGlobal(i));
	}
}
