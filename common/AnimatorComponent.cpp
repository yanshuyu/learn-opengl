#include"AnimatorComponent.h"
#include"Skeleton.h"
#include"Model.h"

COMPONENT_IDENTIFIER_IMP(AnimatorComponent, "AnimatorComponent");


template std::shared_ptr<TConditionVariable<int>> AnimatorComponent::addConditionVar<int>(const std::string& name);
template std::shared_ptr<TConditionVariable<float>> AnimatorComponent::addConditionVar<float>(const std::string& name);
template std::shared_ptr<TConditionVariable<bool>> AnimatorComponent::addConditionVar<bool>(const std::string& name);

template bool AnimatorComponent::setConditionVar<int>(const std::string& name, int val);
template bool AnimatorComponent::setConditionVar<float>(const std::string& name, float val);
template bool AnimatorComponent::setConditionVar<bool>(const std::string& name, bool val);

template std::shared_ptr<TConditionVariable<int>> AnimatorComponent::getConditionVar(const std::string& name);
template std::shared_ptr<TConditionVariable<float>> AnimatorComponent::getConditionVar(const std::string& name);
template std::shared_ptr<TConditionVariable<bool>> AnimatorComponent::getConditionVar(const std::string& name);


AnimatorComponent::AnimatorComponent(): m_avatar()
, m_animatedPose()
, m_curState()
, m_curAnimProgress()
, m_startState()
, m_anyState()
, m_states()
, m_condVars() {
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
	AnimationTransition* tansition = m_curState->firstActiveTransition();
	if (tansition) {
		m_animatedPose = strongAvatar->getSkeleton()->getResPose();
		m_curAnimProgress = 0.f;
		performTransition(tansition);
		return;
	}
	m_curAnimProgress = m_curState->update(m_animatedPose, dt);
}

void AnimatorComponent::performTransition(AnimationTransition* t) {
	m_curState->onExit();
	m_curState = t->getDestinationState();
	m_curState->onEnter();
	
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
std::shared_ptr<TConditionVariable<T>> AnimatorComponent::addConditionVar(const std::string& name) {
	if (m_condVars.find(name) == m_condVars.end()) {
		m_condVars.insert({ name, std::shared_ptr<ConditionVariable>(new TConditionVariable<T>(name)) });
	}

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
std::shared_ptr<TConditionVariable<T>> AnimatorComponent::getConditionVar(const std::string& name) {
	if (m_condVars.find(name) == m_condVars.end())
		return nullptr;

	return std::static_pointer_cast<TConditionVariable<T>>(m_condVars.at(name));
}


bool AnimatorComponent::removeConditionVar(const std::string& name) {
	auto found = m_condVars.find(name);
	if (found == m_condVars.end())
		return false;

	m_condVars.erase(found);
	
	return true;
}