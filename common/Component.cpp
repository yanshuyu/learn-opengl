#include"Component.h"
#include"SceneObject.h"


Component::Component():m_isEnable(true) {

}


void Component::removeFromOwner() const {
	if (m_owner)
		m_owner->removeComponent(this->identifier());
}