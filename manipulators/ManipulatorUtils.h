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

	Line3D() = default;
	Line3D(const vec3& directionlIn, const vec3& originIn) :
		origin(originIn), direction(directionlIn){}

	vec3 projectPoint(vec3 &worldPos);
};


bool LineIntersectPlane(vec3& intersection, const Plane& plane, const Line3D& line);
Line3D MouseToRay(const mat4& cameraModelMatrix, float fov, float aspect, const vec2& normalizedMousePos);
vec2 WorldToNdc(const vec3& pos, const mat4& ViewProj);
float PointToSegmentDistance(const vec2& p0, const vec2& p1, const vec2& ndc);
vec2 NdcToScreen(const vec2& pos, uint w, uint h);
float WorldDistance(const mat4& ViewProj, const mat4& worldTransform);

#endif // MANIPULATORUTILS_H
