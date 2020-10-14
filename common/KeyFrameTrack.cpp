#include"KeyFrameTrack.h"
#include"Util.h"
#include<glm/gtx/quaternion.hpp>

template KeyFrameTrack<float>;
template KeyFrameTrack<glm::vec3>;
template KeyFrameTrack<glm::quat>;


template<typename T>
KeyFrameTrack<T>::KeyFrameTrack(InterpolationType interpolation) : m_interpolationType(interpolation) {
	
}

template<typename T>
KeyFrameTrack<T>::KeyFrameTrack(const KeyFrameTrack<T>& other) {
	*this = other;
}

template<typename T>
KeyFrameTrack<T>::KeyFrameTrack(KeyFrameTrack<T>&& rv) {
	*this = std::move(rv);
}

template<typename T>
KeyFrameTrack<T>& KeyFrameTrack<T>::operator = (const KeyFrameTrack& other) {
	if (this == &other)
		return *this;

	resize(other.size());
	m_interpolationType = other.m_interpolationType;
	std::memcpy(m_keyFrames.data(), other.m_keyFrames.data(), other.size() * sizeof(T));
	
	return *this;
}

template<typename T>
KeyFrameTrack<T>& KeyFrameTrack<T>::operator = (KeyFrameTrack<T>&& rv) {
	m_interpolationType = rv.m_interpolationType;
	std::swap(m_keyFrames, rv.m_keyFrames);
	
	return *this;
}

template<typename T>
float KeyFrameTrack<T>::getStartTime() const {
	if (!isValid())
		return 0.f;
	
	return m_keyFrames[0].m_time;
}

template<typename T>
float KeyFrameTrack<T>::getEndTime() const {
	if (!isValid())
		return 0.f;
	
	return m_keyFrames[m_keyFrames.size() - 1].m_time;
}

template<typename T>
float KeyFrameTrack<T>::getDuration() const {
	if (size() < 2)
		return 0.f;

	return m_keyFrames[m_keyFrames.size() - 1].m_time - m_keyFrames[0].m_time;
}

template<typename T>
bool KeyFrameTrack<T>::isValid() const {
	return getDuration() > 0.f;
}

template<typename T>
T KeyFrameTrack<T>::sample(float t, LoopType loop) const	{
	if (!isValid())
		return T();

	int frameIdx = -1;
	float weight = 0.f;
	auto timeAndSign = ajustTimeToFitTrack(t, loop);
	bool found = findMixFramesFromAjustedTime(timeAndSign.first * timeAndSign.second, frameIdx, weight);

	if (!found) {
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG
		return T();
	}

	if (weight >= 1.f)
		return m_keyFrames[frameIdx].m_value;

	switch (m_interpolationType)
	{
	case InterpolationType::Constant:
		return sampleConstant(frameIdx, weight);

	case InterpolationType::Linear:
		return sampleLinear(frameIdx, weight);

	case InterpolationType::Cubic:
		return sampleCubic(frameIdx,  weight);

	default:
#ifdef _DEBUG
		ASSERT(false);
#endif // _DEBUG

		return T();
	}
}


template<typename T>
T KeyFrameTrack<T>::sampleConstant(int frameIdx, float weight) const {
	return m_keyFrames[frameIdx].m_value;
}

//template<typename T>
//T KeyFrameTrack<T>::sampleLinear(int frameIdx, float weight) const {
//	ASSERT(false); // no implementation
//	return T();
//}

template<>
float ScalarTrack::sampleLinear(int frameIdx, float weight) const {
	return lerp(m_keyFrames[frameIdx].m_value, m_keyFrames[frameIdx + 1].m_value, 1.f - weight);
}

template<>
glm::vec3 VectorTrack::sampleLinear(int frameIdx, float weight) const {
	return lerp(m_keyFrames[frameIdx].m_value, m_keyFrames[frameIdx + 1].m_value, 1.f - weight);
}

template<>
glm::quat QuaternionTrack::sampleLinear(int frameIdx, float weight) const {
	return nlerp(m_keyFrames[frameIdx].m_value, m_keyFrames[frameIdx + 1].m_value, 1.f - weight);
}

template<typename T>
T KeyFrameTrack<T>::sampleCubic(int frameIdx, float weight) const {
	float detal = m_keyFrames[frameIdx + 1].m_time - m_keyFrames[frameIdx].m_time;
	return hermite(m_keyFrames[frameIdx].m_value, 
					m_keyFrames[frameIdx].m_out * detal,
					m_keyFrames[frameIdx+1].m_value,
					m_keyFrames[frameIdx+1].m_in * detal,
					1.f - weight);
}


template<typename T>
std::pair<float, int> KeyFrameTrack<T>::ajustTimeToFitTrack(float t, LoopType loop) const {
	float startTime = getStartTime();
	if (t < startTime)
		return std::make_pair(startTime, 1);

	float endTime = getEndTime();
	if (loop == LoopType::NoLoop)
		return std::make_pair(MIN(t, endTime), 1);

	// normal loop
	float duration = getDuration();
	float relativeTime = t - startTime;
	float remainder = fmodf(relativeTime, duration);

	if (remainder < 0.f)
		remainder += duration;

	if (loop == LoopType::Loop)
		return std::make_pair(remainder + startTime, 1);

	// pingpong loop
	int loopCnt = relativeTime / duration;
	bool forward = loopCnt % 2 == 0; 
	float time = forward ? remainder + startTime : endTime - remainder;
	int sign = forward ? 1 : -1;
	
	return std::make_pair(time, sign);
}


template<typename T>
bool KeyFrameTrack<T>::findMixFramesFromAjustedTime(float time, int& frameIdx, float& mixWeight) const {
	bool backWard = time < 0;
	time = fabs(time);

	frameIdx = -1;
	mixWeight = 0.f;
	
	if (m_interpolationType == InterpolationType::Constant) {
		for (int i = m_keyFrames.size() - 1; i >= 0; i--) {
			if (m_keyFrames[i].m_time <= time) {
				frameIdx = backWard ? i + 1 : i;
				frameIdx = MIN(frameIdx, m_keyFrames.size() - 1);
				mixWeight = 1.f;
				return true;
			}
		}
		
		return false;
	}

	for (int i = m_keyFrames.size() - 1; i >= 0; i--) {
		if (epslion_equal(m_keyFrames[i].m_time, time)) {
			frameIdx = i;
			mixWeight = 1.f;
			break;

		} else if (m_keyFrames[i].m_time < time) {
			frameIdx = i;
			break;
		}
	}

	if (frameIdx < 0)
		return false;
	
	if (mixWeight >= 1.f) // not need to mix
		return true;

	if (frameIdx >= m_keyFrames.size() - 1)
		return false;
	
	int nextFrameIdx = frameIdx + 1;
	mixWeight = 1.f - (time - m_keyFrames[frameIdx].m_time) / (m_keyFrames[nextFrameIdx].m_time - m_keyFrames[frameIdx].m_time);
	
	return true;
}
