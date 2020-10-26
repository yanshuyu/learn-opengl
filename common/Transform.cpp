#include"Transform.h"
#include"Interpolation.h"
#include<glm/gtx/matrix_decompose.hpp>

Transform::Transform(const glm::vec3& p, const glm::vec3& s, const glm::quat& r) :position(p)
, scale(s)
, rotation(r) {

}

Transform combine(const Transform& t1, const Transform& t2) {
	Transform result;

	result.scale = t1.scale * t2.scale;
	result.rotation = t2.rotation * t1.rotation;
	result.position = t1.rotation * (t1.scale * t2.position);
	result.position += t1.position;
	
	return result;
}

Transform inverse(const Transform& t) {
	Transform result;
	result.rotation = glm::inverse(t.rotation);
	result.scale.x = epslion_zero(t.scale.x) ? 0.f : 1.f / t.scale.x;
	result.scale.y = epslion_zero(t.scale.y) ? 0.f : 1.f / t.scale.y;
	result.scale.z = epslion_zero(t.scale.z) ? 0.f : 1.f / t.scale.z;
	result.position = result.rotation * (result.scale * -t.position);

	return result;
}

glm::mat4 transform2Mat(const Transform& t) {
	// First, extract the rotation basis of the transform
	glm::vec3 x = t.rotation * glm::vec3(1.f, 0.f, 0.f);
	glm::vec3 y = t.rotation * glm::vec3(0.f, 1.f, 0.f);
	glm::vec3 z = t.rotation * glm::vec3(0.f, 0.f, 1.f);

	// Next, scale the basis vectors
	x *= t.scale.x;
	y *= t.scale.y;
	z *= t.scale.z;

	// Extract the position of the transform
	glm::vec3 p = t.position;

	return glm::mat4(x.x, x.y, x.z, 0.f, // x basic & scale
					y.x, y.y, y.z, 0.f,	// y baisc & scale
					z.x, z.y, z.z, 0.f, // z basic & scale
					p.x, p.y, p.z, 1.f); // translation
}

Transform mat2Transform(const glm::mat4& m) {
	Transform result;
	glm::vec3 skew(0.f);
	glm::vec4 pespect(0.f);
	glm::decompose(m, result.scale, result.rotation, result.position, skew, pespect);
	
	return result;
}