#include"Interpolation.h"
#include"Pose.h"
#include<glm/gtx/norm.hpp>
#include<glm/gtx/vector_angle.hpp>
#include<glm/gtx/quaternion.hpp>



bool epslion_zero(float v) {
	return fabsf(v) < FLT_EPSILON;
}

bool epslion_zero(const glm::vec3& v) {
	return glm::all(glm::lessThan(glm::abs(v), glm::vec3(FLT_EPSILON)));
}

bool epslion_equal(float a, float b) {
	return epslion_zero(a - b);
}

bool epslion_equal(const glm::vec3& a, const glm::vec3& b) {
	return epslion_zero(a - b);
}


float lerp(float a, float b, float t) {
	return a * (1 - t) + b * t;
}

glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float t) {
	return a + (b - a) * t;
}

glm::vec3 nlerp(const glm::vec3& a, const glm::vec3& b, float t) {
	return glm::normalize(lerp(a, b, t));
}

glm::vec3 slerp(const glm::vec3& a, const glm::vec3& b, float t) {
	if (t < 0.01)
		return nlerp(a, b, t);
	
	float theta = glm::angle(glm::normalize(a), glm::normalize(b));
	float sin_theta = sinf(theta);

	float _a = sinf((1.0f - t) * theta) / sin_theta;
	float _b = sinf(t * theta) / sin_theta;

	return a * _a + b * _b;
}

glm::quat lerp(const glm::quat& q1, const glm::quat& q2, float t) {
	return q1 * (1 - t) + q2 * t;
}

glm::quat nlerp(const glm::quat& q1, const glm::quat& q2, float t) {
	return glm::fastMix(q1, q2, t);
}


template<typename T> T hermite_imp(const T& p1, const T& s1, const T& p2, const T& s2, float t) {
	float tt = t * t;
	float ttt = tt * t;

	float h1 = 2.0f * ttt - 3.0f * tt + 1.0f;
	float h2 = -2.0f * ttt + 3.0f * tt;
	float h3 = ttt - 2.0f * tt + t;
	float h4 = ttt - tt;

	return (p1 * h1) + (p2 * h2) + (s1 * h3) + (s2 * h4);
}


float hermite(float p1, float s1, float p2, float s2, float t) {
	return hermite_imp(p1, s1, p2, s2, t);
}

glm::vec3 hermite(const glm::vec3& p1, const glm::vec3& s1, const glm::vec3& p2, const glm::vec3& s2, float t) {
	return hermite_imp(p1, s1, p2, s2, t);
}

glm::quat hermite(const glm::quat& p1, const glm::quat& s1, const glm::quat& p2, const glm::quat& s2, float t) {
	glm::quat _p2 = p2;
	if (glm::dot(p1, _p2) < 0) // neighborhood
		_p2 = -_p2;
	
	glm::quat reslut = hermite_imp(p1, s1, _p2, s2, t);
	
	return glm::normalize(reslut);
}


void blend(Pose& outPose, const Pose& a, const Pose& b, float t) {
	if (t <= 0) {
		outPose = a;
		return;
	}

	if (t >= 1) {
		outPose = b;
		return;
	}

	outPose = a;

	for (size_t i = 0; i < a.size(); i++) {
		outPose[i].position = lerp(a[i].position, b[i].position, t);
		outPose[i].scale = lerp(a[i].scale, b[i].scale, t);
		outPose[i].rotation = nlerp(a[i].rotation, b[i].rotation, t);
	}
}


void addtiveBlend(Pose& outPose, const Pose& pose, const Pose& addPose, const Pose& addBase) {
	if (&outPose != &pose)
		outPose = pose;

	for (size_t i = 0; i < pose.size(); i++) {
		outPose[i].position = pose[i].position + (addPose[i].position - addBase[i].position);
		outPose[i].scale = pose[i].scale + (addPose[i].scale - addBase[i].scale);
		outPose[i].rotation = glm::normalize(pose[i].rotation * (glm::inverse(addBase[i].rotation) * addPose[i].rotation));
	}
}