#pragma once
#include<glm/glm.hpp>
#include<glm/gtx/quaternion.hpp>

struct Transform {
	glm::vec3 position;
	glm::vec3 scale;
	glm::quat rotation;

	Transform(const glm::vec3& p = glm::vec3(0.f), const glm::vec3& s = glm::vec3(1.f), const glm::quat& r = glm::quat());
};


Transform combine(const Transform& t1, const Transform& t2);
Transform inverse(const Transform& t);
glm::mat4 transform2Mat(const Transform& t);
Transform mat2Transform(const glm::mat4& m);

