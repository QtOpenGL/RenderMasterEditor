#ifndef MANIPULATORUTILS_H
#define MANIPULATORUTILS_H
#include <VectorMath.h>
#include "common.h"
#include <QDebug>


struct Plane
{
	vec3 origin;
	vec3 normal;

	Plane(const vec3& normalIn, const vec3& originIn) :
		origin(originIn), normal(normalIn){}
};


struct Line3D
{
	vec3 origin;
	vec3 direction;

	Line3D(const vec3& directionlIn, const vec3& originIn) :
		origin(originIn), direction(directionlIn){}
};


bool LineIntersectPlane(vec3& intersection, const Plane& plane, const Line3D& line);

Line3D MouseToRay(const mat4& cameraModelMatrix, float fov, float aspect, const vec2& normalizedMousePos);

vec2 WorldToNdc(const vec3& pos, const mat4& Proj);

#endif // MANIPULATORUTILS_H
