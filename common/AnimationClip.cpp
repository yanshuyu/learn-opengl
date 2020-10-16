#include"AnimationClip.h"

AnimationClip::AnimationClip(const std::string& name, float duration): m_name(name)
, m_duration(duration) {

}

Pose& AnimationClip::sample(Pose& ref, float time, LoopType loop) {
	if (!isValid())
		return ref;

	for (auto& track : m_jointsTrack) {
		ref[track.getJointId()] = track.sample(ref[track.getJointId()], time, loop);
	}

	return ref;
}

bool AnimationClip::isValid() const {
	bool isValid = false;
	for (auto& track : m_jointsTrack) {
		if (track.isValid()) {
			isValid = true;
			break;
		}
	}

	return isValid;
}