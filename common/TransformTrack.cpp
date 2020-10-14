#include"TransformTrack.h"
#include"Util.h"

TransformTrack::TransformTrack(int jointId) : m_jointId(jointId) {

}

float TransformTrack::getStartTime() const {
	float t1 = m_positionTrack.getStartTime();
	float t2 = m_scaleTrack.getStartTime();
	float t3 = m_rotationTrack.getStartTime();

	float result = MIN(t1, t2);
	result = MIN(result, t3);
	
	return result;
}

float TransformTrack::getEndTime() const {
	float t1 = m_positionTrack.getEndTime();
	float t2 = m_scaleTrack.getEndTime();
	float t3 = m_rotationTrack.getEndTime();

	float result = MAX(t1, t2);
	result = MAX(result, t3);

	return result;
}

float TransformTrack::getDuration() const {
	return getEndTime() - getStartTime();
}

bool TransformTrack::isValid() const {
	bool posValid = m_positionTrack.isValid();
	bool scaleValid = m_scaleTrack.isValid();
	bool rotateValid = m_rotationTrack.isValid();
	
	return posValid || scaleValid || rotateValid;
}

Transform TransformTrack::sample(const Transform& ref, float time, LoopType loop) const {
	if (!isValid())
		return ref;
	
	Transform result = ref;
	if (m_positionTrack.isValid())
		result.position = m_positionTrack.sample(time, loop);
	if (m_scaleTrack.isValid())
		result.scale = m_scaleTrack.sample(time, loop);
	if (m_rotationTrack.isValid())
		result.rotation = m_rotationTrack.sample(time, loop);

	return result;
}