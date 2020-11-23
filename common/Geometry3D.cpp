#include"Geometry3D.h"
#include"Util.h"
#include<glm/gtx/norm.hpp>
#include<cmath>
#include<array>

Ray Line::toRay() const {
	return Ray::FromPoints(start, end);
}

float Length(const Line& l) {
	return glm::distance(l.start, l.end);
}

float LengthSq(const Line& l) {
	return glm::distance2(l.start, l.end);
}


Ray Ray::FromPoints(const Point& o, const Point& target) {
	return Ray(o, target - o);
}


AABB AABB::fromMinMax(const Point& min, const Point& max) {
	return AABB((min + max)*0.5f, max - min);
}


std::pair<Point, Point> AABB::getMinMax() const {
	Point min = center - halfExtends;
	Point max = center + halfExtends;
	
	min.x = MIN(min.x, max.x);
	min.y = MIN(min.y, max.y);
	min.z = MIN(min.z, max.z);
	max.x = MAX(min.x, max.x);
	max.y = MAX(min.y, max.y);
	max.z = MAX(min.z, max.z);

	return { min, max };
}


Plane Plane::FromTriangle(const Triangle& triangle) {
	Plane plane;
	plane.normal = glm::normalize(glm::cross(triangle.b - triangle.a, triangle.c - triangle.a));
	plane.distance = glm::dot(triangle.a, plane.normal);

	return plane;
}


void Plane::normalize() {
	float k = 1.f / glm::length(normal);
	normal *= k;
	distance *= k;
}


Plane Plane::normalized() const {
	Plane p(*this);
	p.normalize();
	
	return p;
}


Plane::FaceSide Plane::getFaceSide(const Point& point) const {
	float d = glm::dot(point, normal) - distance;
	
	if (FLT_CMP(d, 0.f))
		return FaceSide::NoSide;

	if (d < 0.f)
		return FaceSide::Back;

	return FaceSide::Front;
}


Plane::FaceSide Plane::getFaceSide(const Sphere& sphere) const {
	float d = glm::dot(sphere.position, normal) - distance;

	if (fabsf(d) <= sphere.radius)
		return FaceSide::NoSide;

	if (d < 0.f)
		return FaceSide::Back;

	return FaceSide::Front;
}


Plane::FaceSide Plane::getFaceSide(const AABB& aabb) const {
	// project aabbb half extends to plane normal
	float pe = aabb.halfExtends.x * fabsf(normal.x) + aabb.halfExtends.y * fabsf(normal.y) + aabb.halfExtends.z * fabsf(normal.z);
	
	// project aabb center to plane normal, get relative distance to plane
	float pc = glm::dot(aabb.center, normal) - distance;

	if (fabsf(pc) <= pe)
		return FaceSide::NoSide; 

	if (pc < 0)
		return FaceSide::Back;

	return FaceSide::Front;
}


Plane::FaceSide Plane::getFaceSide(const OBB& obb) const {
	// project obb half extends to plane normal in obb local space
	glm::vec3 normalL = glm::transpose(obb.rotation) * normal; // obb.rotation * normal ??
	float pe = obb.halfExtends.x * fabsf(normalL.x) + obb.halfExtends.y * fabsf(normalL.y) + obb.halfExtends.z * fabsf(normalL.z);

	// project obb center to plane normal, get relative distance to plane
	float pc = glm::dot(obb.center, normal) - distance;

	if (fabsf(pc) <= pe)
		return FaceSide::NoSide;

	if (pc < 0)
		return FaceSide::Back;

	return FaceSide::Front;
}


float planeEquation(const Plane& plane, const Point& p) {
	return glm::dot(p, plane.normal) - plane.distance;
}


Frustum Frustum::FromMatrix(const glm::mat4& m) {
	Frustum frustum;
	glm::vec4 row1 = { m[0][0], m[1][0], m[2][0], m[3][0] };
	glm::vec4 row2 = { m[0][1], m[1][1], m[2][1], m[3][1] };
	glm::vec4 row3 = { m[0][2], m[1][2], m[2][2], m[3][2] };
	glm::vec4 row4 = { m[0][3], m[1][3], m[2][3], m[3][3] };

	frustum.left.normal = row4 + row1;
	frustum.left.distance = -(row4 + row1).w;
	
	frustum.right.normal = row4 - row1;
	frustum.right.distance = -(row4 - row1).w;
	
	frustum.bottom.normal = row4 + row2;
	frustum.bottom.distance = -(row4 + row2).w;

	frustum.top.normal = row4 - row2;
	frustum.top.distance = -(row4 - row2).w;

	frustum.near.normal = row4 + row3;
	frustum.near.distance = -(row4 + row3).w;

	frustum.far.normal = row4 - row3;
	frustum.far.distance = -(row4 - row3).w;

	for (auto& plane : frustum.planes) {
		plane.normalize();
	}

	return frustum;
}

std::array<Point, 8> Frustum::getCorners() const {
	std::array<Point, 8> corners;
	corners[0] = Intersection(near, left, bottom).second;
	corners[1] = Intersection(near, left, top).second;
	corners[2] = Intersection(near, right, top).second;
	corners[3] = Intersection(near, right, bottom).second;

	corners[4] = Intersection(far, left, bottom).second;
	corners[5] = Intersection(far, left, top).second;
	corners[6] = Intersection(far, right, top).second;
	corners[7] = Intersection(far, right, bottom).second;

	return corners;
}


std::pair<bool, Point> Intersection(const Plane& plane1, const Plane& plane2, const Plane& plane3) {
	glm::mat3 m(plane1.normal.x, plane1.normal.y, plane1.normal.z,
		plane2.normal.x, plane2.normal.y, plane2.normal.z,
		plane3.normal.x, plane3.normal.y, plane3.normal.z);
	float det = glm::determinant(m);

	if (det == 0)
		return std::make_pair(false, PointZero);

	glm::vec3 d(plane1.distance, plane2.distance, plane3.distance);
	glm::mat3 invM = glm::inverse(m);

	return std::make_pair(true, invM * d);
}


bool PointInsideSphere(const Point& point, const Sphere& sphere) {
	return LengthSq(Line(sphere.position, point)) <= sphere.radius * sphere.radius;
}


Point ClosestPoint(const Sphere& sphere, const Point& point) {
	return sphere.position + glm::normalize(point - sphere.position) * sphere.radius;
}


bool PointInsideAABB(const Point& point, const AABB& aabb) {
	auto minMax = aabb.getMinMax();
	auto& min = minMax.first;
	auto& max = minMax.second;

	if (point.x < min.x || point.y < min.y || point.z < min.z)
		return false;

	if (point.x > max.x || point.y > max.y || point.z > max.z)
		return false;

	return true;
}


Point ClosestPoint(const AABB& aabb, const Point& point) {
	Point result = point;
	auto minMax = aabb.getMinMax();
	auto& min = minMax.first;
	auto& max = minMax.second;

	result.x = MAX(result.x, min.x);
	result.y = MAX(result.y, min.y);
	result.z = MAX(result.z, min.z);
	result.x = MIN(result.x, max.x);
	result.y = MIN(result.y, max.y);
	result.z = MIN(result.z, max.z);

	return result;
}


bool PointInsideOBB(const Point& point, const OBB& obb) {
	// project point to each local axis of obb
	// test each projection whether in range of obb extend
	glm::vec3 op = point - obb.center;
	
	for (size_t i = 0; i < 3; i++) {
		float proj = glm::dot(op, obb.rotation[i]);
		if (proj > obb.halfExtends[i] || proj < -obb.halfExtends[i])
			return false;
	}

	return true;
}


Point ClosestPoint(const OBB& obb, const Point& point) {
	Point result = obb.center;
	glm::vec3 op = point - obb.center;

	for (size_t i = 0; i < 3; i++) {
		float proj = glm::dot(op, obb.rotation[i]);
		proj = MAX(proj, -obb.halfExtends[i]);
		proj = MIN(proj, obb.halfExtends[i]);
		result += obb.rotation[i] * proj;
	}

	return result;
}


bool PointOnPlane(const Point& point, const Plane& plane) {
	return FLT_CMP(planeEquation(plane, point), 0.f);
}


Point ClosestPoint(const Plane& plane, const Point& point) {
	float proj = glm::dot(point, plane.normal);
	return point + (plane.distance - proj) * plane.normal;
}


bool PointOnLine(const Point& point, const Line& line) {
	Point closest = ClosestPoint(line, point);
	return FLT_CMP(glm::distance2(point, closest), 0.f);
}


Point ClosestPoint(const Line& line, const Point& point) {
	glm::vec3 lVec = line.end - line.start;
	float t = glm::dot(point - line.start, lVec) / glm::dot(lVec, lVec);
	t = clamp(t, 0.f, 1.f);
	
	return line.start + lVec * t;
}


bool PointOnRay(const Ray& ray, const Point& point) {
	glm::vec3 op = glm::normalize(point - ray.origin);
	return FLT_CMP(glm::dot(op, ray.direction), 1.f);
}


Point ClosestPoint(const Ray& ray, const Point& point) {
	float t = MAX(glm::dot(point - ray.origin, ray.direction), 0.f);
	return ray.origin + ray.direction * t;
}


bool PointInTriangle(const Point& point, const Triangle& triangle) {
	// create a pyramid which tip is point
	// if normals of each side face of pyramid all point to the same direction
	// there is a intersection
	glm::vec3 pa = triangle.a - point;
	glm::vec3 pb = triangle.b - point;
	glm::vec3 pc = triangle.c - point;
	glm::vec3 normPAB = glm::cross(pa, pb);
	glm::vec3 normPBC = glm::cross(pb, pc);
	glm::vec3 normPCA = glm::cross(pc, pa);

	if (glm::dot(normPAB, normPBC) < 0.f)
		return false;

	if (glm::dot(normPBC, normPCA) < 0.f)
		return false;

	return true;
}


Point ClosestPoint(const Triangle& triangle, const Point& point) {
	Plane plane = Plane::FromTriangle(triangle);
	Point closest = ClosestPoint(plane, point);
	
	if (PointInTriangle(closest, triangle))
		return closest;

	// closest on one of triangle edge
	Point points[3] = {
		ClosestPoint(Line(triangle.a, triangle.b), closest),
		ClosestPoint(Line(triangle.b, triangle.c), closest),
		ClosestPoint(Line(triangle.c, triangle.a), closest),
	};

	float distances[3] = {
		glm::distance2(points[0], closest),
		glm::distance2(points[1], closest),
		glm::distance2(points[2], closest),
	};
	
	float minDistance = distances[0];
	int idx = 0;
	for (int i = 0; i < 3; i++) {
		if (distances[i] < minDistance) {
			minDistance = distances[i];
			idx = i;
		}
	}

	return points[idx];
}


bool PointInFrustum(const Point& point, const Frustum& frustum) {
	for (auto& plane : frustum.planes) {
		float detalDistance = glm::dot(point, plane.normal) - plane.distance;
		if (detalDistance < 0)
			return false;
	}

	return true;
}


bool SphereSphere(const Sphere& s1, const Sphere& s2) {
	return glm::distance2(s1.position, s2.position) <= (s1.radius + s2.radius) * (s1.radius + s2.radius);
}


bool SphereAABB(const Sphere& sphere, const AABB& aabb) {
	Point closest = ClosestPoint(aabb, sphere.position);
	return glm::distance2(sphere.position, closest) <= sphere.radius * sphere.radius;
}


bool SphereOBB(const Sphere& sphere, const OBB& obb) {
	Point closest = ClosestPoint(obb, sphere.position);
	return glm::distance2(sphere.position, closest) <= sphere.radius * sphere.radius;
}


bool SpherePlane(const Sphere& sphere, const Plane& plane) {
	Point closest = ClosestPoint(plane, sphere.position);
	return glm::distance2(sphere.position, closest) <= sphere.radius * sphere.radius;
}


bool AABBAABB(const AABB& aabb1, const AABB& aabb2) {
	auto aMinMax = aabb1.getMinMax();
	auto bMinMax = aabb2.getMinMax();
	auto& aMin = aMinMax.first;
	auto& aMax = aMinMax.second;
	auto& bMin = bMinMax.first;
	auto& bMax = bMinMax.second;

	return (aMin.x < bMax.x&& bMin.x < aMax.x)
		&& (aMin.y < bMax.y&& bMin.y < aMax.y)
		&& (aMin.z < bMax.z&& bMin.z < aMax.z);
}


bool AABBOBB(const AABB& aabb, const OBB& obb) {
	// aabb obb SAT test
	glm::vec3 aabbXBase = { 1.f, 0.f, 0.f };
	glm::vec3 aabbYBase = { 0.f, 1.f, 0.f };
	glm::vec3 aabbZBase = { 0.f, 0.f, 1.f };
	glm::vec3 obbXBase = obb.rotation[0];
	glm::vec3 obbYBase = obb.rotation[1];
	glm::vec3 obbZBase = obb.rotation[2];

	glm::vec3 axies[15] = {};
	axies[0] = aabbXBase;
	axies[1] = aabbYBase;
	axies[2] = aabbZBase;
	axies[3] = obbXBase;
	axies[4] = obbYBase;
	axies[5] = obbZBase;
	axies[6] = glm::cross(aabbXBase, obbXBase);
	axies[7] = glm::cross(aabbXBase, obbYBase);
	axies[8] = glm::cross(aabbXBase, obbZBase);
	axies[9] = glm::cross(aabbYBase, obbXBase);
	axies[10] = glm::cross(aabbYBase, obbYBase);
	axies[11] = glm::cross(aabbYBase, obbZBase);
	axies[12] = glm::cross(aabbZBase, obbXBase);
	axies[13] = glm::cross(aabbZBase, obbYBase);
	axies[14] = glm::cross(aabbZBase, obbZBase);

	for (auto& axis : axies) {
		if (!OverlapOnAxis(aabb, obb, axis))
			return false;
	}

	return true;
}


bool AABBPlane(const AABB& aabb, const Plane& plane) {
	// solution
	/*
	Interval proj = getInterval(aabb, plane.normal);
	return proj.min < plane.distance && plane.distance < proj.max;
	*/
	
	// optional solution
	return plane.getFaceSide(aabb) == Plane::FaceSide::NoSide;
}


bool OBBOBB(const OBB& obb1, const OBB& obb2) {
	const glm::vec3& obb1XBase = obb1.rotation[0];
	const glm::vec3& obb1YBase = obb1.rotation[1];
	const glm::vec3& obb1ZBase = obb1.rotation[2];
	const glm::vec3& obb2XBase = obb2.rotation[0];
	const glm::vec3& obb2YBase = obb2.rotation[1];
	const glm::vec3& obb2ZBase = obb2.rotation[2];

	glm::vec3 axies[15] = {};
	axies[0] = obb1XBase;
	axies[1] = obb1YBase;
	axies[2] = obb1ZBase;
	axies[3] = obb2XBase;
	axies[4] = obb2YBase;
	axies[5] = obb2ZBase;
	axies[6] = glm::cross(obb1XBase, obb2XBase);
	axies[7] = glm::cross(obb1XBase, obb2YBase);
	axies[8] = glm::cross(obb1XBase, obb2ZBase);
	axies[9] = glm::cross(obb1YBase, obb2XBase);
	axies[10] = glm::cross(obb1YBase, obb2YBase);
	axies[11] = glm::cross(obb1YBase, obb2ZBase);
	axies[12] = glm::cross(obb1ZBase, obb2XBase);
	axies[13] = glm::cross(obb1ZBase, obb2YBase);
	axies[14] = glm::cross(obb1ZBase, obb2ZBase);

	for (auto& axis : axies) {
		if (!OverlapOnAxis(obb1, obb2, axis))
			return false;
	}

	return true;
}


bool OBBPlane(const OBB& obb, const Plane& plane) {
	// solutiom
	/* 
	Interval proj = getInterval(obb, plane.normal);
	return proj.min < plane.distance && plane.distance < proj.max;
	*/

	// optional solution
	return plane.getFaceSide(obb) == Plane::FaceSide::NoSide;
}


bool PlanePlane(const Plane& plane1, const Plane& plane2) {
	float dot = fabsf(glm::dot(plane1.normal, plane2.normal));
	return !FLT_CMP(dot, 1.f);
}


bool TriangleSphere(const Triangle& triangle, const Sphere& sphere) {
	Point closest = ClosestPoint(triangle, sphere.position);
	return glm::distance2(closest, sphere.position) <= sphere.radius * sphere.radius;
}


bool TriangleAABB(const Triangle& triangle, const AABB& aabb) {
	glm::vec3 u1(1.f, 0.f, 0.f); // xyz axies of aabb
	glm::vec3 u2(0.f, 1.f, 0.f);
	glm::vec3 u3(0.f, 0.f, 1.f);
	glm::vec3 e1 = triangle.b - triangle.a; // edges of triangle
	glm::vec3 e2 = triangle.c - triangle.a;
	glm::vec3 e3 = triangle.c - triangle.b;
	glm::vec3 n = glm::cross(e1, e2); //face normal of triangle

	glm::vec3 axies[13] = {
		u1, u2, u3,
		n,
		glm::cross(u1, e1), glm::cross(u1, e2), glm::cross(u1, e3),
		glm::cross(u2, e1), glm::cross(u2, e2), glm::cross(u2, e3),
		glm::cross(u3, e1), glm::cross(u3, e2), glm::cross(u3, e3),
	};

	for (auto& axis : axies) {
		if (!OverlapOnAxis(aabb, triangle, axis))
			return false;
	}

	return true;
}


bool TriangleOBB(const Triangle& triangle, const OBB& obb) {
	glm::vec3 u1 = obb.rotation[0]; // xyz axies of obb
	glm::vec3 u2 = obb.rotation[1];
	glm::vec3 u3 = obb.rotation[2];
	glm::vec3 e1 = triangle.b - triangle.a; // edges of triangle
	glm::vec3 e2 = triangle.c - triangle.a;
	glm::vec3 e3 = triangle.c - triangle.b;
	glm::vec3 n = glm::cross(e1, e2); //face normal of triangle

	glm::vec3 axies[13] = {
		u1, u2, u3,
		n,
		glm::cross(u1, e1), glm::cross(u1, e2), glm::cross(u1, e3),
		glm::cross(u2, e1), glm::cross(u2, e2), glm::cross(u2, e3),
		glm::cross(u3, e1), glm::cross(u3, e2), glm::cross(u3, e3),
	};

	for (auto& axis : axies) {
		if (!OverlapOnAxis(obb, triangle, axis))
			return false;
	}

	return true;
}


bool TrianglePlane(const Triangle& triangle, const Plane& plane) {
	float d1 = planeEquation(plane, triangle.a);
	float d2 = planeEquation(plane, triangle.b);
	float d3 = planeEquation(plane, triangle.c);

	if (FLT_CMP(d1, 0.f) && FLT_CMP(d2, 0.f) && FLT_CMP(d3, 0.f)) // points of triangle on plane
		return true;

	if (d1 > 0 && d2 > 0 && d3 > 0) // points of trianle on side side of plane
		return false;

	if (d1 < 0 && d2 < 0 && d3 < 0)
		return false;

	return true;
}


bool TriangleTriangle(const Triangle& triangle1, const Triangle& triangle2) {
	glm::vec3 e11 = triangle1.b - triangle1.a; // edges of triangle1
	glm::vec3 e12 = triangle1.c - triangle1.a;
	glm::vec3 e13 = triangle1.c - triangle1.b;

	glm::vec3 e21 = triangle2.b - triangle2.a; // edges of triangle2
	glm::vec3 e22 = triangle2.c - triangle2.a;
	glm::vec3 e23 = triangle2.c - triangle2.b;

	glm::vec3 axies[11] = {
		glm::cross(e11, e12), glm::cross(e21, e22), // face normals of triangle
		glm::cross(e11, e21), glm::cross(e11, e22), glm::cross(e11, e23),
		glm::cross(e12, e21), glm::cross(e12, e22), glm::cross(e12, e23),
		glm::cross(e13, e21), glm::cross(e13, e22), glm::cross(e13, e23),
	};

	for (auto& axis : axies) {
		if (!OverlapOnAxis(triangle1, triangle2, axis))
			return false;
	}

	return true;
}


bool FrustumSphere(const Frustum& frustum, const Sphere& sphere) {
	for (auto& plane : frustum.planes) {
		if (plane.getFaceSide(sphere) == Plane::FaceSide::Back)
			return false;
	}

	return true;
}


bool FrustumAABB(const Frustum& frustum, const AABB& aabb) {
	for (auto& plane : frustum.planes) {
		if (plane.getFaceSide(aabb) == Plane::FaceSide::Back)
			return false;
	}

	return true;
}


bool FrustumOBB(const Frustum& frustum, const OBB& obb) {
	for (auto& plane : frustum.planes) {
		if (plane.getFaceSide(obb) == Plane::FaceSide::Back)
			return false;
	}

	return true;
}


Interval getInterval(const AABB& aabb, const glm::vec3& axis) {
	// project 8 vertices of aabb to axis
	auto minMax = aabb.getMinMax();
	auto& min = minMax.first;
	auto& max = minMax.second;

	Point points[8] = {
		{min.x, min.y, min.z},
		{min.x, max.y, min.z},
		{max.x, max.y, min.z},
		{max.x, min.y, min.z},
		{min.x, min.y, max.z},
		{min.x, max.y, max.z},
		{max.x, max.y, max.z},
		{max.x, min.y, max.z},
	};

	Interval result;
	result.min = result.max = glm::dot(points[0], axis);
	for (auto& p : points) {
		float proj = glm::dot(p, axis);
		result.min = MIN(result.min, proj);
		result.max = MAX(result.max, proj);
	}

	return result;
}



Interval getInterval(const OBB& obb, const glm::vec3& axis) {
	// project 8 vertices of obb to axis
	const glm::vec3& xBase = obb.rotation[0];
	const glm::vec3& yBase = obb.rotation[1];
	const glm::vec3& zBase = obb.rotation[2];
	Point min = -obb.halfExtends;
	Point max = obb.halfExtends;
	Point points[8] = {};
	
	points[0] = obb.center + (min.x * xBase + min.y * yBase + min.z * zBase);
	points[1] = obb.center + (min.x * xBase + max.y * yBase + min.z * zBase);
	points[2] = obb.center + (max.x * xBase + max.y * yBase + min.z * zBase);
	points[3] = obb.center + (max.x * xBase + min.y * yBase + min.z * zBase);
	points[4] = obb.center + (min.x * xBase + min.y * yBase + max.z * zBase);
	points[5] = obb.center + (min.x * xBase + max.y * yBase + max.z * zBase);
	points[6] = obb.center + (max.x * xBase + max.y * yBase + max.z * zBase);
	points[7] = obb.center + (max.x * xBase + min.y * yBase + max.z * zBase);

	Interval result;
	result.min = result.max = glm::dot(points[0], axis);
	for (auto& p : points) {
		float proj = glm::dot(p, axis);
		result.min = MIN(result.min, proj);
		result.max = MAX(result.max, proj);
	}

	return result;
}


Interval getInterval(const Triangle& triangle, const glm::vec3& axis) {
	Interval result;
	result.min = result.max = glm::dot(triangle.points[0], axis);
	for (size_t i = 1; i < 3; i++) {
		float proj = glm::dot(triangle.points[i], axis);
		result.min = MIN(result.min, proj);
		result.max = MAX(result.max, proj);
	}

	return result;
}


bool OverlapOnAxis(const AABB& aabb, const Triangle& triangle, const glm::vec3& axis) {
	Interval a = getInterval(aabb, axis);
	Interval b = getInterval(triangle, axis);

	return a.min <= b.max && b.min <= a.max;
}


bool OverlapOnAxis(const AABB& aabb, const OBB& obb, const glm::vec3& axis) {
	Interval a = getInterval(aabb, axis);
	Interval b = getInterval(obb, axis);

	return a.min <= b.max&& b.min <= a.max;
}


bool OverlapOnAxis(const OBB& obb1, const OBB& obb2, const glm::vec3& axis) {
	Interval a = getInterval(obb1, axis);
	Interval b = getInterval(obb2, axis);

	return a.min <= b.max&& b.min <= a.max;
}


bool OverlapOnAxis(const OBB& obb, const Triangle& triangle, const glm::vec3& axis) {
	Interval a = getInterval(obb, axis);
	Interval b = getInterval(triangle, axis);

	return a.min <= b.max && b.min <= a.max;
}


bool OverlapOnAxis(const Triangle& triangle1, const Triangle& triangle2, const glm::vec3& axis) {
	Interval a = getInterval(triangle1, axis);
	Interval b = getInterval(triangle2, axis);

	return a.min <= b.max && b.min <= a.max;
}



float RayCast(const Ray& ray, const Sphere& sphere) {
	glm::vec3 op = sphere.position - ray.origin;
	float oqLen = glm::dot(op, ray.direction);
	float opLenSq = glm::length2(op);
	float pqLenSq = opLenSq - oqLen * oqLen;
	float rLenSq = sphere.radius * sphere.radius;

	if (pqLenSq > rLenSq)
		return -1;

	float i = std::sqrtf(rLenSq - pqLenSq);

	if (opLenSq <= rLenSq) // ray cast inside sphere
		return oqLen + i;

	return oqLen - i;
}


float RayCast(const Ray& ray, const AABB& aabb) {
	auto minMax = aabb.getMinMax();
	auto& min = minMax.first;
	auto& max = minMax.second;

	float t1 = (min.x - ray.origin.x) / (FLT_CMP(ray.direction.x, 0.0f) ? 0.00001f : ray.direction.x);
	float t2 = (max.x - ray.origin.x) / (FLT_CMP(ray.direction.x, 0.0f) ? 0.00001f : ray.direction.x);
	float t3 = (min.y - ray.origin.y) / (FLT_CMP(ray.direction.y, 0.0f) ? 0.00001f : ray.direction.y);
	float t4 = (max.y - ray.origin.y) / (FLT_CMP(ray.direction.y, 0.0f) ? 0.00001f : ray.direction.y);
	float t5 = (min.z - ray.origin.z) / (FLT_CMP(ray.direction.z, 0.0f) ? 0.00001f : ray.direction.z);
	float t6 = (max.z - ray.origin.z) / (FLT_CMP(ray.direction.z, 0.0f) ? 0.00001f : ray.direction.z);

	float tmin = fmaxf(fmaxf(fminf(t1, t2), fminf(t3, t4)), fminf(t5, t6));
	float tmax = fminf(fminf(fmaxf(t1, t2), fmaxf(t3, t4)), fmaxf(t5, t6));

	if (tmax < 0) // ray is intersecting aabb in negative direction
		return -1;

	if (tmin > tmax)
		return -1;

	if (tmin < 0) // ray is intersecting aabb and origin is inside aabb
		return tmax;

	return tmin;
}


float RayCast(const Ray& ray, const OBB& obb) {
	glm::vec3 p = obb.center - ray.origin;
	const glm::vec3& X = obb.rotation[0];
	const glm::vec3& Y = obb.rotation[1];
	const glm::vec3& Z = obb.rotation[2];

	glm::vec3 f(
		glm::dot(X, ray.direction),
		glm::dot(Y, ray.direction),
		glm::dot(Z, ray.direction)
	);

	glm::vec3 e(
		glm::dot(X, p),
		glm::dot(Y, p),
		glm::dot(Z, p)
	);

	float t[6] = { 0, 0, 0, 0, 0, 0 };
	for (int i = 0; i < 3; ++i) {
		if (FLT_CMP(f[i], 0)) { // ray is parallel to slab
			if (-e[i] - obb.halfExtends[i] > 0 || -e[i] + obb.halfExtends[i] < 0) { //ray is outside of aabb
				return -1;
			}
			f[i] = 0.00001f; // ray is inside aabb, Avoid div by 0!
		}

		t[i * 2 + 0] = (e[i] + obb.halfExtends[i]) / f[i]; // tmin[x, y, z]
		t[i * 2 + 1] = (e[i] - obb.halfExtends[i]) / f[i]; // tmax[x, y, z]
	}

	float tmin = fmaxf(fmaxf(fminf(t[0], t[1]), fminf(t[2], t[3])), fminf(t[4], t[5]));
	float tmax = fminf(fminf(fmaxf(t[0], t[1]), fmaxf(t[2], t[3])), fmaxf(t[4], t[5]));

	if (tmax < 0) // ray is intersecting obb in negative direction
		return -1;

	if (tmin > tmax)
		return -1;

	if (tmin < 0) // ray is intersecting obb and origin is inside aabb
		return tmax;

	return tmin;
}



float RayCast(const Ray& ray, const Plane& plane) {
	float nd = glm::dot(ray.direction, plane.normal);

	if (nd >= 0.f) // ray and plane's normal point to the same direction, ray intersect plane from back side
		return -1;

	float pn = glm::dot(ray.origin, plane.normal);
	float t = (plane.distance - pn) / nd;
	
	if (t < 0) // ray hit plane behind origin
		return -1; 

	return t;
}



float RayCast(const Ray& ray, const Triangle& triangle) {
	float t = RayCast(ray, Plane::FromTriangle(triangle));
	if (t < 0)
		return t;

	if (PointInTriangle(ray.origin + ray.direction * t, triangle))
		return t;

	return -1;
}



bool LineTest(const Line& line, const Sphere& sphere) {
	Point closest = ClosestPoint(line, sphere.position);
	return glm::distance2(sphere.position, closest) <= sphere.radius * sphere.radius;
}


bool LineTest(const Line& line, const AABB& aabb) {
	float t = RayCast(line.toRay(), aabb);
	return t >= 0 && t * t < LengthSq(line);
}


bool LineTest(const Line& line, const OBB& obb) {
	float t = RayCast(line.toRay(), obb);
	return t >= 0 && t * t < LengthSq(line);
}


bool LineTest(const Line& line, const Plane& plane) {
	glm::vec3 ab = line.end - line.start;
	float sn = glm::dot(line.start, plane.normal);
	float abn = glm::dot(ab, plane.normal);

	if (FLT_CMP(abn, 0.f)) // line is parallel with plane
		return false;

	float t = (plane.distance - sn) / abn;
	
	return t >= 0.f && t <= 1.f;
}


bool LineTest(const Line& line, const Triangle& triangle) {
	float t = RayCast(line.toRay(), triangle);
	if (t < 0)
		return t;

	return t > 0 && t * t <= LengthSq(line);
}


