#include"AnimationPlayer.h"
#include"AnimationState.h"
#include"Interpolation.h"


AnimationPlayer::AnimationPlayer(): m_playItems()
, m_curState(nullptr)
, m_prevState(nullptr)
, m_fadeDuration(0.f)
, m_fadeTimer(0.f)
, m_curTimer(0.f)
, m_prevTimer(0.f)
, m_isPaused(false)
, m_speed(1.f) {

}

void AnimationPlayer::jumpTo(AnimationState* state) {
	while (!m_playItems.empty()) {
		m_playItems.pop();
	}
	replaceBy(state);
}

void AnimationPlayer::fadeTo(AnimationState* state, float duration, bool immediately) {
	if (!m_curState) {
		jumpTo(state);
		return;
	}

	if (immediately) {
		while (!m_playItems.empty()) {
			m_playItems.pop();
		}
	}

	m_playItems.push(PlayItem(state, duration));
}

float AnimationPlayer::update(Pose& outPose, float dt) {
	if (!m_curState)
		return 0.f;

	bool fading = isFading();

	if (fading && m_fadeTimer >= m_fadeDuration) {
		m_prevState = nullptr;
		m_prevTimer = 0.f;
		m_fadeDuration = 0.f;
		m_fadeTimer = 0.f;
		fading = false;
	}

	if (!fading && !m_isPaused && !m_playItems.empty()) {
		auto& next = m_playItems.front();
		if (next.fadeDuration <= 0.f) {
			replaceBy(next.targetState);
		}
		else {
			transitionTo(next.targetState, next.fadeDuration);
			fading = true;
		}
		m_playItems.pop();
	}
	
	if (fading) {
		m_prevState->update(m_prevState->m_refPose, m_prevTimer);
		float progress = m_curState->update(m_curState->m_refPose, m_curTimer);

		blend(outPose, m_prevState->m_refPose, m_curState->m_refPose, m_fadeTimer / m_fadeDuration);

		if (!m_isPaused) {
			m_prevTimer += dt * m_prevState->m_speed * m_speed;
			m_curTimer += dt * m_curState->m_speed * m_speed;
			m_fadeTimer += dt * m_speed;
		}

		return progress;
	}
	
	
	float progress = m_curState->update(outPose, m_curTimer);
	
	if (!m_isPaused) {
		m_curTimer += dt * m_curState->m_speed * m_speed;
	}

	return progress;
}


bool AnimationPlayer::isFading() {
	return m_prevState != nullptr
		&& m_curState != nullptr
		&& m_fadeDuration > 0.f;
}


void AnimationPlayer::replaceBy(AnimationState* state) {
	m_prevState = nullptr;
	m_curState = state;
	m_prevTimer = 0.f;
	m_curTimer = 0.f;
	m_fadeDuration = 0.f;
	m_fadeTimer = 0.f;
}


void AnimationPlayer::transitionTo(AnimationState* state, float dur) {
	m_prevState = m_curState;
	m_prevTimer = m_curTimer;
	m_curState = state;
	m_curTimer = 0.f;
	m_fadeDuration = dur;
	m_fadeTimer = 0.f;
}