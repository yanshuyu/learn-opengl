#pragma once
#include"Component.h"


class RenderContext;


class RenderableComponent : public Component {

	RTTI_DECLARATION(RenderableComponent)

public:
	RenderableComponent();
	virtual ~RenderableComponent() {}

	virtual void render(RenderContext* context) = 0;

};