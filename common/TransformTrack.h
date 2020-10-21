#pragma once
#include"KeyFrameTrack.h"
#include"Transform.h"

// transform tack contain lower level track to fully descript 
// a joint's tranform curve
class TransformTrack {
public:
	TransformTrack(int jointId = -1);
	TransformTrack(const TransformTrack& other) = default;
	TransformTrack(TransformTrack&& rv) = default;

	TransformTrack& operator = (const TransformTrack& other) = default;
	TransformTrack& operator = (TransformTrack&& rv) = default;

	float getStartTime() const;
	float getEndTime() const;
	float getDuration() const;
	bool isValid() const;

	void sample(Transform& outTransform, float time, LoopType loop) const;

	inline VectorTrack& getPositionTrack() {
		return m_positionTrack;
	}

	inline VectorTrack& getScaleTrack() {
		return m_scaleTrack;
	}

	inline QuaternionTrack& getRotationTrack() {
		return m_rotationTrack;
	}

	inline int getJointId() const {
		return m_jointId;
	}

	inline void setJointId(int idx) {
		m_jointId = idx;
	}

protected:
	int m_jointId; // index of skeleton joint array
	VectorTrack m_positionTrack;
	VectorTrack m_scaleTrack;
	QuaternionTrack m_rotationTrack;
};