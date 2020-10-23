#pragma once
#include"AnimationState.h"
#include"Pose.h"
#include"Component.h"
#include"Util.h"
#include"AnimationPlayer.h"
#include<unordered_map>
#include<memory>


class Model;
class Pose;

// animator contain collection of states and conditional variable that control
// transition between states. any state should update avatar's animated pose.
class AnimatorComponent: public Component {
public:
	AnimatorComponent();

	COMPONENT_IDENTIFIER_DEC;

	inline std::string identifier() const override {
		return s_identifier;
	}
	virtual Component* copy() const override;
	
	void setAvater(std::weak_ptr<Model> avatar);
	void update(double dt) override;
	void reset();

	// states management
	AnimationState* addState(const std::string& name);
	AnimationState* getState(const std::string& name);
	bool removeState(const std::string& name);
	
	void setInitialState(const std::string& name);
	void setInitialState(AnimationState* state);

	// condition variables management
	template<typename T>
	std::shared_ptr<TConditionVariable<T>> addConditionVar(const std::string& name, T val = T());

	template<typename T>
	bool setConditionVar(const std::string& name, T val);

	template<typename T>
	std::shared_ptr<TConditionVariable<T>> getConditionVar(const std::string& name);
	
	bool removeConditionVar(const std::string& name);

	//getter & setter
	inline const AnimationState* currentState() const {
		return m_curState;
	}
	
	inline AnimationState* anyState() const {
		return m_anyState.get();
	}

	inline float currentAnimationProgress() const {
		return m_curAnimProgress;
	}

	inline std::weak_ptr<Model> getAvatar() const {
		return m_avatar;
	}

	inline const Pose& animatedPose() const {
		return m_animatedPose;
	}

protected:
	void removeAllTransitionToState(AnimationState* state);
	bool isInitState(AnimationState* state);

protected:
	std::weak_ptr<Model> m_avatar;
	Pose m_animatedPose;

	std::unique_ptr<AnimationState> m_startState;
	std::shared_ptr<BoolConditionVar> m_startToInitCond;

	std::unique_ptr<AnimationState> m_anyState;

	AnimationState* m_curState;
	float m_curAnimProgress;
	AnimationPlayer m_player;

	std::unordered_map<std::string, std::unique_ptr<AnimationState>> m_states;
	std::unordered_map<std::string, std::shared_ptr<ConditionVariable>> m_condVars;
};