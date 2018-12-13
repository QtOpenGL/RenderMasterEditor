#ifndef MANIPULATORUTILS_H
#define MANIPULATORUTILS_H
#include <VectorMath.h>


struct Plane
{
	vec3 origin;
	vec3 normal;

	Plane(const vec3& originIn, const vec3& normalIn) :
		origin(originIn), normal(normalIn){}
};


//struct Line3D
//{
//	vec3 origin;
//	vec3 direction;

//	Line3D(const vec3& originIn, const vec3& directionlIn) :
//		origin(originIn), direction(directionlIn){}
//};


//bool LineIntersectPlane(vec3& intersection, const Plane& plane, const Line3D& line)
//{
//	// assuming vectors are all normalized
//	float denom = plane.normal.Dot(line.direction);
//	if (denom > 1e-6) {
//		vec3 p0l0 = plane.origin - line.origin;
//		float t = p0l0.Dot(plane.normal) / denom;
//		if (t >= 0)
//		{
//			intersection = line.origin + line.direction * t;
//			return true;
//		}
//		return false;
//	}

//	return false;
//}


#endif // MANIPULATORUTILS_H
