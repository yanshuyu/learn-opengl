#pragma once
#include<string>


class SceneObject;
class RenderContext;

class Component {
	friend class SceneObject;

public:
	Component();
	virtual ~Component() {}
	
	virtual bool initialize() { return true; }
	virtual void update(double dt) {}
	virtual void render(RenderContext* context) {}

	virtual std::string identifier() const = 0;
	virtual Component* copy() const = 0;

	void removeFromOwner() const;
	SceneObject* owner() const {
		return m_owner;
	}

	bool m_isEnable;

protected:
	SceneObject* m_owner;
};