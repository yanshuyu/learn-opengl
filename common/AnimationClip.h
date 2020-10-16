#pragma once
#include"TransformTrack.h"
#include"Pose.h"
#include<string>
#include<vector>



// a animation clip contain collection of joints transform tracks, to overwrite all 
// or part of  bind pose's joints transform, sample a clip result a animated pose
class AnimationClip {
public:
	AnimationClip(const std::string& name = "", float duration = 0.f);

	Pose& sample(Pose& ref, float time, LoopType loop);

	bool isValid() const;

	inline TransformTrack& trackAt(size_t idx) {
		return m_jointsTrack[idx];
	}

	inline size_t size() const {
		return m_jointsTrack.size();
	}

	inline void resize(size_t sz) {
		m_jointsTrack.resize(sz);
	}

	inline std::vector<TransformTrack>& getTracks() {
		return m_jointsTrack;
	}

	inline std::string getName() const {
		return m_name;
	}

	inline void setName(const std::string& name) {
		m_name = name;
	}

	inline float getDuration() const {
		return m_duration;
	}

	inline void setDuration(float duration) {
		m_duration = duration;
	}

protected:
	std::vector<TransformTrack> m_jointsTrack;
	std::string m_name;
	float m_duration;
};