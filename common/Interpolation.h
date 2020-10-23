#pragma once
#include<glm/glm.hpp>

class Pose;

enum class LoopType {
	NoLoop,
	Loop,
	PingPong,
};

enum class InterpolationType {
	Constant,
	Linear,
	Cubic,
};

bool epslion_zero(float v);

bool epslion_zero(const glm::vec3& v);

bool epslion_equal(float a, float b);

bool epslion_equal(const glm::vec3& a, const glm::vec3& b);

//
// linear interpolation
//
float lerp(float a, float b, float t);

glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float t);

glm::vec3 nlerp(const glm::vec3& a, const glm::vec3& b, float t);

glm::vec3 slerp(const glm::vec3& a, const glm::vec3& b, float t);

glm::quat lerp(const glm::quat& q1, const glm::quat& q2, float t);

glm::quat nlerp(const glm::quat& q1, const glm::quat& q2, float t);


//
// cubic interpolation
//
float hermite(float p1, float s1, float p2, float s2, float t);

glm::vec3 hermite(const glm::vec3& p1, const glm::vec3& s1, const glm::vec3& p2, const glm::vec3& s2, float t);

glm::quat hermite(const glm::quat& p1, const glm::quat& s1, const glm::quat& p2, const glm::quat& s2, float t);



//
// animation blending
//

void blend(Pose& outPose, const Pose& a, const Pose& b, float t);

void addtiveBlend(Pose& outPose, const Pose& pose, const Pose& addPose, const Pose& addBase);