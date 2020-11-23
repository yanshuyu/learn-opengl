#pragma once
#include<glm/glm.hpp>
#include<utility>

#define PointZero Point(0.f)
#define IPointZero IPoint(0)
#define NoRotation glm::mat3(1.f)

typedef glm::vec3 Point;
typedef glm::ivec3 IPoint;


struct Ray;

// line is shortest distance between two points
/// extend infinit in two direction
struct Line {
	Point start;
	Point end;

	Ray toRay() const;

	inline Line() : Line(PointZero, {0.f, 0.f, 1.f}) { }
	inline Line(const Point& s, const Point& e): start(s), end(e) { }
};

float Length(const Line& l);
float LengthSq(const Line& l);



struct Ray {
	Point origin;
	glm::vec3 direction;
	
	static Ray FromPoints(const Point& o, const Point& target);

	inline Ray() :Ray(PointZero, {0.f, 0.f, 1.f}) { }
	inline Ray(const Point& o, const glm::vec3& d):origin(o), direction(d) {
		direction = glm::normalize(direction);
	}
};



struct Sphere {
	Point position;
	float radius;
	
	inline Sphere(): Sphere(1.f) { }
	inline Sphere(float r):Sphere(PointZero, r) { }
	inline Sphere(const Point& p, float r) : position(p), radius(r) { }
};



struct AABB {
	Point center;
	glm::vec3 halfExtends;

	static AABB fromMinMax(const Point& min, const Point& max);
	std::pair<Point, Point> getMinMax() const;

	inline AABB() : AABB({1.f, 1.f, 1.f}) { }
	inline AABB(const glm::vec3& dimension): AABB(PointZero, dimension) { }
	inline AABB(const Point& c, const glm::vec3& dimension) : center(c), halfExtends(dimension * 0.5f) { }
};



struct OBB {
	Point center;
	glm::vec3 halfExtends;
	glm::mat3 rotation;

	inline OBB(): OBB(PointZero) { }
	inline OBB(const Point& c) : OBB(c, { 1.f, 1.f, 1.f }, NoRotation) { }
	inline OBB(const Point& c, const glm::vec3& dimension): OBB(c, dimension, NoRotation) { }
	inline OBB(const Point& c, const glm::vec3& dimension, const glm::mat3& orientation): center(c), halfExtends(dimension*0.5f), rotation(orientation) { }
};


struct Triangle;

struct Plane {
	enum class FaceSide {
		NoSide,
		Front,
		Back,
	};

	glm::vec3 normal;
	float distance; // distance to origin along normal direction

	static Plane FromTriangle(const Triangle& triangle);

	inline Plane() : Plane({ 0.f, 1.f, 0.f }) { }
	inline Plane(const glm::vec3& n) : Plane(n, 0.f) { }
	inline Plane(const glm::vec3& n, float d) : normal(n), distance(d) { 
		normal = glm::normalize(normal);
	}

	void normalize();
	Plane normalized() const;
	FaceSide getFaceSide(const Point& point) const;
	FaceSide getFaceSide(const Sphere& sphere) const;
	FaceSide getFaceSide(const AABB& aabb) const;
	FaceSide getFaceSide(const OBB& obb) const;
	
};

float planeEquation(const Plane& plane, const Point& p);



struct Triangle {
	union {
		struct {
			Point a;
			Point b;
			Point c;
		};

		Point points[3];

		float values[9];
	};

	inline Triangle(): Triangle({ -1.f, 0.f, 0.f }, { 0.f, 0.f, -1.f }, {1.f, 0.f, 0.f}) { }
	inline Triangle(const Point& p0, const Point& p1, const Point& p2) : a(p0), b(p1), c(p2) { }
};


struct Interval { //SAT test
	float min;
	float max;

	inline Interval(): Interval(0.f, 0.f) { }
	inline Interval(float _min, float _max) : min(_min), max(_max) { }
};


struct Frustum
{
	union  {
		struct {
			Plane left;
			Plane right;
			Plane bottom;
			Plane top;
			Plane near;
			Plane far;
		};

		Plane planes[6];
	};

	std::array<Point, 8> getCorners() const;

	static Frustum FromMatrix(const glm::mat4& m);

	Frustum(): planes() {}
};

std::pair<bool, Point> Intersection(const Plane& plane1, const Plane& plane2, const Plane& plane3);

//    
// 3D point test
// point inside volume or on surface of primitives
// cloest point on primitives surface to tested point
//
bool PointInsideSphere(const Point& point, const Sphere& sphere);
bool PointInsideAABB(const Point& point, const AABB& aabb);
bool PointInsideOBB(const Point& point, const OBB& obb);
bool PointOnPlane(const Point& point, const Plane& plane);
bool PointOnLine(const Point& point, const Line& line);
bool PointOnRay(const Ray& ray, const Point& point);
bool PointInTriangle(const Point& point, const Triangle& triangle);
bool PointInFrustum(const Point& point, const Frustum& frustum);


Point ClosestPoint(const Sphere& sphere, const Point& point);
Point ClosestPoint(const AABB& aabb, const Point& point);
Point ClosestPoint(const OBB& obb, const Point& point);
Point ClosestPoint(const Plane& plane, const Point& point);
Point ClosestPoint(const Line& line, const Point& point);
Point ClosestPoint(const Ray& ray, const Point& point);
Point ClosestPoint(const Triangle& triangle, const Point& point);


inline bool ContainPoint(const Sphere& s, const Point& p) { return PointInsideSphere(p, s); }
inline bool ContainPoint(const AABB& aabb, const Point& p) { return PointInsideAABB(p, aabb); }
inline bool ContainPoint(const OBB& obb, const Point& p) { return PointInsideOBB(p, obb); }
inline bool ContainPoint(const Plane& plane, const Point& p) { return PointOnPlane(p, plane); }
inline bool ContainPoint(const Line& l, const Point& p) { return PointOnLine(p, l); }
inline bool ContainPoint(const Ray& ray, const Point& p) { return PointOnRay(ray, p); }
inline bool ContainPoint(const Triangle& t, const Point& p) { return PointInTriangle(p, t); }
inline bool ContainPoint(const Frustum& f, const Point& p) { return PointInFrustum(p, f); }


//
// 3D primitives collision detection
//
bool SphereSphere(const Sphere& s1, const Sphere& s2);
bool SphereAABB(const Sphere& sphere, const AABB& aabb);
bool SphereOBB(const Sphere& sphere, const OBB& obb);
bool SpherePlane(const Sphere& sphere, const Plane& plane);
bool AABBAABB(const AABB& aabb1, const AABB& aabb2);
bool AABBOBB(const AABB& aabb, const OBB& obb);
bool AABBPlane(const AABB& aabb, const Plane& plane);
bool OBBOBB(const OBB& obb1, const OBB& obb2);
bool OBBPlane(const OBB& obb, const Plane& plane);
bool PlanePlane(const Plane& plane1, const Plane& plane2);
bool TriangleSphere(const Triangle& triangle, const Sphere& sphere);
bool TriangleAABB(const Triangle& triangle, const AABB& aabb);
bool TriangleOBB(const Triangle& triangle, const OBB& obb);
bool TrianglePlane(const Triangle& triangle, const Plane& plane);
bool TriangleTriangle(const Triangle& triangle1, const Triangle& triangle2);
bool FrustumSphere(const Frustum& frustum, const Sphere& sphere);
bool FrustumAABB(const Frustum& frustum, const AABB& aabb);
bool FrustumOBB(const Frustum& frustum, const OBB& obb);

inline bool Intersects(const Sphere& s1, const Sphere& s2) { return SphereSphere(s1, s2); }
inline bool Intersects(const Sphere& s, const AABB& aabb) { return SphereAABB(s, aabb); }
inline bool Intersects(const AABB& aabb, const Sphere& s) { return SphereAABB(s, aabb); }
inline bool Intersects(const Sphere& s, const OBB& obb) { return SphereOBB(s, obb); }
inline bool Intersects(const OBB& obb, const Sphere& s) { return SphereOBB(s, obb); }
inline bool Intersects(const Sphere& s, const Plane& p) { return SpherePlane(s, p); }
inline bool Intersects(const Plane& p, const Sphere& s) { return SpherePlane(s, p); }
inline bool Intersects(const AABB& aabb1, const AABB& aabb2) { return AABBAABB(aabb1, aabb2); }
inline bool Intersects(const AABB& aabb, const OBB& obb) { return AABBOBB(aabb, obb); }
inline bool Intersects(const OBB& obb, const AABB& aabb) { return AABBOBB(aabb, obb); }
inline bool Intersects(const AABB& aabb, const Plane& p) { return AABBPlane(aabb, p); }
inline bool Intersects(const Plane& p, const AABB& aabb) { return AABBPlane(aabb, p); }
inline bool Intersects(const OBB& obb1, const OBB& obb2) { return OBBOBB(obb1, obb2); }
inline bool Intersects(const OBB& obb, const Plane& p) { return OBBPlane(obb, p); }
inline bool Intersects(const Plane& p, const OBB& obb) { return OBBPlane(obb, p); }
inline bool Intersects(const Plane& p1, const Plane& p2) { return PlanePlane(p1, p2); }
inline bool Intersects(const Triangle& t, const Sphere& s) { return TriangleSphere(t, s); }
inline bool Intersects(const Sphere& s, const Triangle& t) { return TriangleSphere(t, s); }
inline bool Intersects(const Triangle& t, const AABB& aabb) { return TriangleAABB(t, aabb); }
inline bool Intersects(const AABB& aabb, const Triangle& t) { return TriangleAABB(t, aabb); }
inline bool Intersects(const Triangle& t, const OBB& obb) { return TriangleOBB(t, obb); }
inline bool Intersects(const OBB& obb, const Triangle& t) { return TriangleOBB(t, obb); }
inline bool Intersects(const Triangle& t, const Plane& p) { return TrianglePlane(t, p); }
inline bool Intersects(const Plane& p, const Triangle& t) { return TrianglePlane(t, p); }
inline bool Intersects(const Triangle& t1, const Triangle& t2) { return TriangleTriangle(t1, t2); }
inline bool Intersects(const Frustum& f, const Sphere& s) { return FrustumSphere(f, s); }
inline bool Intersects(const Sphere& s, const Frustum& f) { return FrustumSphere(f, s); }
inline bool Intersects(const Frustum& f, const AABB& aabb) { return FrustumAABB(f, aabb); }
inline bool Intersects(const AABB& aabb, const Frustum& f) { return FrustumAABB(f, aabb); }
inline bool Intersects(const Frustum& f, const OBB& obb) { return FrustumOBB(f, obb); }
inline bool Intersects(const OBB& obb, const Frustum& f) { return FrustumOBB(f, obb); }



//
// Ray cast test
//
float RayCast(const Ray& ray, const Sphere& sphere);
float RayCast(const Ray& ray, const AABB& aabb);
float RayCast(const Ray& ray, const OBB& obb);
float RayCast(const Ray& ray, const Plane& plane);
float RayCast(const Ray& ray, const Triangle& triangle);


//
// Line test
//
bool LineTest(const Line& line, const Sphere& sphere);
bool LineTest(const Line& line, const AABB& aabb);
bool LineTest(const Line& line, const OBB& obb);
bool LineTest(const Line& line, const Plane& plane);
bool LineTest(const Line& line, const Triangle& triangle);


//
// SAT helper (seperate axis theory)
//
Interval getInterval(const AABB& aabb, const glm::vec3& axis);
Interval getInterval(const OBB& obb, const glm::vec3& axis);
Interval getInterval(const Triangle& triangle, const glm::vec3& axis);

bool OverlapOnAxis(const AABB& aabb, const OBB& obb, const glm::vec3& axis);
bool OverlapOnAxis(const OBB& obb1, const OBB& obb2, const glm::vec3& axis);
bool OverlapOnAxis(const AABB& aabb, const Triangle& triangle, const glm::vec3& axis);
bool OverlapOnAxis(const OBB& obb, const Triangle& triangle, const glm::vec3& axis);
bool OverlapOnAxis(const Triangle& triangle1, const Triangle& triangle2, const glm::vec3& axis);
