#include"AnimationClip.h"
#include"Util.h"

AnimationClip::AnimationClip(const std::string& name, float duration): m_name(name)
, m_duration(duration) {

}

float AnimationClip::sample(Pose& outPose, float time, LoopType loop) {
	if (!isValid())
		return 0.f;

	float ajustedTime = ajustTimeToFitClip(time, loop);
	for (auto& track : m_jointsTrack) {
		track.sample(outPose[track.getJointId()], ajustedTime, loop);
	}

	return ajustedTime / getDuration();
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


float AnimationClip::getStartTime() const {
	float t = 0.f;
	for (auto& track : m_jointsTrack) {
		t = MIN(track.getStartTime(), t);
	}

	return t;
}

float AnimationClip::getEndTime() const {
	return getStartTime() + m_duration;
}


float AnimationClip::ajustTimeToFitClip(float time, LoopType loop) {
	float startTime = getStartTime();
	if (time < startTime)
		return startTime;

	float endTime = getEndTime();
	if (loop == LoopType::NoLoop)
		return MIN(time, endTime);

	// normal loop
	float duration = getDuration();
	float relativeTime = time - startTime;
	float remainder = fmodf(relativeTime, duration);

	if (remainder < 0.f)
		remainder += duration;

	if (loop == LoopType::Loop)
		return remainder + startTime;

	// pingpong loop
	int loopCnt = relativeTime / duration;
	bool forward = loopCnt % 2 == 0;
	
	return forward ? remainder + startTime : endTime - remainder;
}