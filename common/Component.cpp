#include"Component.h"
#include"SceneObject.h"
#include"Scene.h"


RTTI_IMPLEMENTATION(Component)

Component::Component():m_isEnable(true)
, m_owner(nullptr){

}


void Component::removeFromOwner() const {
	if (m_owner)
		m_owner->removeComponent(s_typeId);
}


void Component::onAttached() {
#ifdef _DEBUG
	ASSERT(m_owner);
#endif // _DEBUG
	m_owner->getParentScene()->onComponentAttach(m_owner, this);
}

void Component::onDetached() {
#ifdef _DEBUG
	ASSERT(m_owner);
#endif // _DEBUG
	m_owner->getParentScene()->onComponentDetach(m_owner, this);
}