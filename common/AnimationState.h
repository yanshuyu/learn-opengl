#pragma once
#include"AnimationTransition.h"
#include"Interpolation.h"
#include<string>
#include<memory>


class Pose;
class AnimationClip;
class Skeleton;

// state contain state specsific data 
// and collection of possible transitions to other state.
class AnimationState {
public:
	AnimationState(const std::string& name = "");
	AnimationState(const AnimationState& other) = delete;
	AnimationState& operator = (const AnimationState& other) = delete;

	// life cycle
	void onEnter();
	void onExit();
	float update(Pose& outPose, float dt);

	AnimationTransition* firstActiveTransition() const;

	// transition managment
	void addTransition(AnimationState* dst);
	template<typename T>
	void addTransition(AnimationState* dst, std::weak_ptr<TConditionVariable<T>> conVar, ConditionComparer cmp, T refVar);

	bool removeTransition(AnimationState* dst);
	bool removeTransition(const std::string& dstStateName);

	AnimationTransition* getTransition(AnimationState* dst);
	AnimationTransition* getTransition(const std::string& dstStateName);

	inline size_t transitionCount() const {
		return m_transitions.size();
	}

	inline AnimationTransition* transitionAt(size_t idx) {
		return m_transitions[idx].get();
	}

	inline void removeAllTransition() {
		m_transitions.clear();
	}

	// setter & getter
	inline void setName(const std::string& name) {
		m_name = name;
	}

	inline std::string getName() const {
		return m_name;
	}

	inline void setAnimationClip(AnimationClip* clip) {
		m_clip = clip;
	}

	inline AnimationClip* getAnimationClip() const {
		return m_clip;
	}

	inline void setAnimationLoopType(LoopType loop) {
		m_loopMode = loop;
	}

	inline LoopType getAnimationLoopType() const {
		return m_loopMode;
	}

protected:
	int getTransitionIdx(const std::string& dstName) const;

protected:
	std::string m_name;
	std::vector<std::unique_ptr<AnimationTransition>> m_transitions; //out transitions

	AnimationClip* m_clip;
	LoopType m_loopMode;
	float m_timer;
	float m_progress;
};