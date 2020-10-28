#include"Component.h"
#include"SceneObject.h"


RTTI_IMPLEMENTATION(Component)

Component::Component():m_isEnable(true) {

}


void Component::removeFromOwner() const {
	if (m_owner)
		m_owner->removeComponent(s_typeId);
}