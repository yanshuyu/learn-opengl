#pragma once
#include<glm/glm.hpp>
#include"RendererCore.h"

class ShaderProgram;


class ShadowMapping {
public:
	~ShadowMapping() {};

	virtual bool initialize() = 0;
	virtual	void cleanUp() {};
	virtual void beginShadowPhase(const Light_t& light, const Camera_t& camera) = 0;
	virtual void endShadowPhase(const Light_t& light) = 0;
	virtual void beginLighttingPhase(const Light_t& light, ShaderProgram* shader) = 0;
	virtual void endLighttingPhase(const Light_t& light, ShaderProgram* shader) = 0;
	virtual void onShadowMapResolutionChange(float w, float h) = 0;
};