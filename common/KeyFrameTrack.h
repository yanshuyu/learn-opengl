#pragma once
#include"KeyFrame.h"
#include"Interpolation.h"
#include<vector>


// track contain collection of key frames
// and how to intepolate between them
template<typename T>
class KeyFrameTrack {
#ifdef _DEBUG
	friend class DebugDrawer;
#endif // _DEBUG

public:
	KeyFrameTrack(InterpolationType interpolation = InterpolationType::Linear);
	KeyFrameTrack(const KeyFrameTrack& other);
	KeyFrameTrack(KeyFrameTrack&& rv);

	KeyFrameTrack& operator = (const KeyFrameTrack& other);
	KeyFrameTrack& operator = (KeyFrameTrack&& rv);


	T sample(float t, LoopType loop) const;
	
	float getStartTime() const;
	float getEndTime() const;
	float getDuration() const;
	bool isValid() const;

	inline size_t size() const {
		return m_keyFrames.size();
	}

	inline void resize(size_t sz) {
		m_keyFrames.resize(sz);
	}

	inline KeyFrame<T>& operator [] (size_t idx) {
		return m_keyFrames[idx];
	}
	
	inline std::vector<KeyFrame<T>>& getKeyFrames() {
		return m_keyFrames;
	}
	
	inline InterpolationType getInterpolationType() const {
		return m_interpolationType;
	}

	inline void setInterpolationType(InterpolationType interpolation) {
		m_interpolationType = interpolation;
	}

protected:
	T sampleConstant(int frameIdx, float weight) const;
	T sampleLinear(int frameIdx, float weight) const;
	T sampleCubic(int frameIdx, float weight) const;

	std::pair<float, int> ajustTimeToFitTrack(float t, LoopType loop) const;
	bool findMixFramesFromAjustedTime(float time, int& frameIdx, float& mixWeight) const;

protected:
	std::vector<KeyFrame<T>> m_keyFrames;
	InterpolationType m_interpolationType;
};


typedef KeyFrameTrack<float> ScalarTrack;
typedef KeyFrameTrack<glm::vec3> VectorTrack;
typedef KeyFrameTrack<glm::quat> QuaternionTrack;