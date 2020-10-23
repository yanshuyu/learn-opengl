#pragma once
#include"Pose.h"
#include<queue>


class AnimationState;

class AnimationPlayer {
	struct PlayItem {
		AnimationState* targetState;
		float fadeDuration;

		PlayItem(AnimationState* target, float dur) : targetState(target)
		, fadeDuration(dur) {

		}
	};

public:
	AnimationPlayer();
	AnimationPlayer(const AnimationPlayer& other) = delete;
	AnimationPlayer operator = (const AnimationPlayer& other) = delete;

	void jumpTo(AnimationState* state);
	void fadeTo(AnimationState* state, float duration, bool immediately = false);
	float update(Pose& outPose, float dt);

	inline void pause() {
		m_isPaused = true;
	}

	inline void resume() {
		m_isPaused = false;
	}

	inline bool isPause() const {
		return m_isPaused;
	}

	inline void setRate(float rate) {
		m_speed = rate;
	}

	inline float getRate() const {
		return m_speed;
	}

protected:
	bool isFading();
	void replaceBy(AnimationState* state);
	void transitionTo(AnimationState* state, float dur);

protected:
	std::queue<PlayItem> m_playItems;
	AnimationState* m_curState;
	AnimationState* m_prevState;
	float m_curTimer;
	float m_prevTimer;

	float m_fadeDuration;
	float m_fadeTimer;

	float m_speed;
	bool m_isPaused;
};