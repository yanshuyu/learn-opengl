#include"AnimatorComponent.h"
#include"Skeleton.h"
#include"Model.h"

COMPONENT_IDENTIFIER_IMP(AnimatorComponent, "AnimatorComponent");


template std::weak_ptr<TConditionVariable<int>> AnimatorComponent::addConditionVar<int>(const std::string& name, int val);
template std::weak_ptr<TConditionVariable<float>> AnimatorComponent::addConditionVar<float>(const std::string& name, float val);
template std::weak_ptr<TConditionVariable<bool>> AnimatorComponent::addConditionVar<bool>(const std::string& name, bool val);

template bool AnimatorComponent::setConditionVar<int>(const std::string& name, int val);
template bool AnimatorComponent::setConditionVar<float>(const std::string& name, float val);
template bool AnimatorComponent::setConditionVar<bool>(const std::string& name, bool val);

template std::weak_ptr<TConditionVariable<int>> AnimatorComponent::getConditionVar(const std::string& name);
template std::weak_ptr<TConditionVariable<float>> AnimatorComponent::getConditionVar(const std::string& name);
template std::weak_ptr<TConditionVariable<bool>> AnimatorComponent::getConditionVar(const std::string& name);


AnimatorComponent::AnimatorComponent(): m_avatar()
, m_animatedPose()
, m_curState()
, m_curAnimProgress()
, m_startState()
, m_anyState()
, m_states()
, m_condVars()
, m_player() {
	m_startState.reset(new AnimationState("Start"));
	m_startToInitCond = std::shared_ptr<BoolConditionVar>(new BoolConditionVar("Animator@Start", true));
	m_anyState.reset(new AnimationState("Any"));
	m_curState = m_startState.get();
}


Component* AnimatorComponent::copy() const {
	return nullptr;
}


void AnimatorComponent::setAvater(std::weak_ptr<Model> avatar) {
	reset();
	m_avatar = avatar;
	if (!avatar.expired()) {
		m_animatedPose = m_avatar.lock()->getSkeleton()->getResPose();
	}
}

void AnimatorComponent::update(double dt) {
	if (m_avatar.expired()) {
		reset();
		return;
	}

	auto strongAvatar = m_avatar.lock();
	
	AnimationTransition* transition = m_anyState->anyActiveTransition();

	if (transition && m_curState == transition->getDestinationState())
		transition = nullptr;

	if (transition )
		m_anyState->setAnimationClip(m_curState->getAnimationClip());

	if (!transition)
		transition = m_curState->anyActiveTransition();

	if (transition) {
		m_curState->onExit();
		m_animatedPose = strongAvatar->getSkeleton()->getResPose();
		m_curState = transition->getDestinationState();
		m_curState->setPose(m_animatedPose);
		m_curState->onEnter();
		m_player.fadeTo(m_curState, transition->getDuration());
		m_curAnimProgress = 0.f;
	}

	m_curAnimProgress = m_player.update(m_animatedPose, dt);
}


void AnimatorComponent::reset() {
	m_avatar.reset();
	m_states.clear();
	m_condVars.clear();

	m_startState->removeAllTransition();
	m_anyState->removeAllTransition();
	m_curState = m_startState.get();
	m_curAnimProgress = 0.f;
}


AnimationState* AnimatorComponent::addState(const std::string& name) {
	auto found = m_states.find(name);
	if (found != m_states.end())
		return m_states.at(name).get();

	m_states.insert({ name, std::make_unique<AnimationState>(name) });

	if (m_states.size() == 1) { // first add state
		setInitialState(m_states.at(name).get());
	}

	return m_states.at(name).get();
}


AnimationState* AnimatorComponent::getState(const std::string& name) {
	auto found = m_states.find(name);
	if (found != m_states.end())
		return m_states.at(name).get();
	
	return nullptr;
}


bool AnimatorComponent::removeState(const std::string& name) {
	auto found = m_states.find(name);
	if (found != m_states.end()) {
		bool isStart = isInitState(found->second.get());
		removeAllTransitionToState(found->second.get());
		m_states.erase(found);
		
		if (isStart && m_states.size() > 0) {
			setInitialState(m_states.begin()->second.get());
		} 

		return true;
	}

	return false;
}


void AnimatorComponent::setInitialState(const std::string& name) {
	setInitialState(getState(name));
}


void AnimatorComponent::setInitialState(AnimationState* state) {
	if (state) {
		m_startState->removeAllTransition();
		m_startState->addTransition(state, std::weak_ptr<BoolConditionVar>(m_startToInitCond), ConditionComparer::Equal, true);
	}
}


void AnimatorComponent::removeAllTransitionToState(AnimationState* state) {
	for (auto& item : m_states) {
		if (item.second.get() == state)
			continue;
		item.second->removeTransition(state);
	}
	m_startState->removeTransition(state);
	m_anyState->removeTransition(state);
}


bool AnimatorComponent::isInitState(AnimationState* state) {
	if (m_startState->transitionCount() <= 0)
		return false;

	return m_startState->transitionAt(0)->getDestinationState() == state;
}


template<typename T>
std::weak_ptr<TConditionVariable<T>> AnimatorComponent::addConditionVar(const std::string& name, T val) {
	if (m_condVars.find(name) == m_condVars.end()) {
		m_condVars.insert({ name, std::shared_ptr<ConditionVariable>(new TConditionVariable<T>(name)) });
	}

	setConditionVar<T>(name, val);

	return std::static_pointer_cast<TConditionVariable<T>>(m_condVars.at(name));
}


template<typename T>
bool AnimatorComponent::setConditionVar(const std::string& name, T val) {
	auto pos = m_condVars.find(name);
	if (pos == m_condVars.end())
		return false;

	TConditionVariable<T>* var = static_cast<TConditionVariable<T>*>(pos->second.get());
	var->m_vaule = val;

	return true;
}


template<typename T>
std::weak_ptr<TConditionVariable<T>> AnimatorComponent::getConditionVar(const std::string& name) {
	if (m_condVars.find(name) == m_condVars.end())
		return std::weak_ptr<TConditionVariable<T>>();

	return std::static_pointer_cast<TConditionVariable<T>>(m_condVars.at(name));
}


bool AnimatorComponent::removeConditionVar(const std::string& name) {
	auto found = m_condVars.find(name);
	if (found == m_condVars.end())
		return false;

	m_condVars.erase(found);
	
	return true;
}


int AnimatorComponent::animationCount() const {
	if (m_avatar.expired())
		return 0;

	return m_avatar.lock()->getAnimations().size();
}


AnimationClip* AnimatorComponent::animationAt(int idx) const {
	if (m_avatar.expired())
		return nullptr;

	return m_avatar.lock()->getAnimations()[idx];
}


std::vector<AnimationClip*> AnimatorComponent::animationClips() const {
	if (m_avatar.expired())
		return {};

	return std::move(m_avatar.lock()->getAnimations());
}