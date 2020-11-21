#pragma once
#include"RTTI.h"
#include<string>


class SceneObject;

class Component: public RTTI {
	friend class SceneObject;

	RTTI_DECLARATION(Component)

public:
	Component();
	virtual ~Component() {}
	
	virtual bool initialize() { return true; }
	virtual void update(double dt) {}
	virtual Component* copy() const = 0;
	virtual void onAttached();
	virtual void onDetached();

	void removeFromOwner() const;

	SceneObject* owner() const {
		return m_owner;
	}

	bool m_isEnable;

protected:
	SceneObject* m_owner;
};