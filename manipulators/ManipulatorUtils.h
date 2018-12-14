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


struct Line3D
{
	vec3 origin;
	vec3 direction;

	Line3D(const vec3& originIn, const vec3& directionlIn) :
		origin(originIn), direction(directionlIn){}
};


bool LineIntersectPlane(vec3& intersection, const Plane& plane, const Line3D& line);


#endif // MANIPULATORUTILS_H
