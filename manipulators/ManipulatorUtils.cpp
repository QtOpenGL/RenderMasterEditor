#include "ManipulatorUtils.h"
#include <algorithm>

bool LineIntersectPlane(vec3& intersection, const Plane& plane, const Line3D& line)
{
	vec3 R = line.direction.Normalized();
	vec3 N = plane.normal.Normalized();

	float d = N.Dot(plane.origin);

	float denom = N.Dot(R);
	if (abs(denom) < 0.00001f) return false;

	float x = (d - N.Dot(line.origin)) / denom;

	intersection = line.origin + R * x;

	return true;
}

Line3D MouseToRay(const mat4& cameraModelMatrix, float fov, float aspect, const vec2& ndc)
{
    vec3 forwardN = -cameraModelMatrix.Column3(2).Normalized();

    float y = tan(DEGTORAD * fov * 0.5f);
    float x = y;

    vec3 rightN = cameraModelMatrix.Column3(0).Normalized();
    vec3 right = rightN * x * aspect;

    vec3 upN = cameraModelMatrix.Column3(1).Normalized();
    vec3 up = upN * y;

    vec2 mousePos = ndc * 2.0f - vec2(1.0f, 1.0f);

    vec3 dir = (forwardN + right * mousePos.x + up * mousePos.y).Normalized();
	vec3 origin = cameraModelMatrix.Column3(3);

    return Line3D(dir, origin);
}

vec2 WorldToNdc(const vec3& pos, const mat4& ViewProj)
{
    vec4 screenPos = ViewProj * vec4(pos);
    screenPos /= screenPos.w;
    return vec2(screenPos.x, screenPos.y);
}

float PointToSegmentDistance(const vec2& p0, const vec2& p1, const vec2& point)
{
    vec2 direction = vec2(p1.x - p0.x, p1.y - p0.y);
    vec2 p00 = p0;
    return (p00 + direction * clamp(direction.Dot(point - p00) / direction.Dot(direction), 0.0f, 1.0f) - point).Lenght();
}

vec2 NdcToScreen(const vec2 &ndc, uint w, uint h)
{
    vec2 tmp = ndc * 0.5f + vec2(0.5f, 0.5f);
    return vec2(tmp.x * w, tmp.y * h);
}

float DistanceTo(const mat4& ViewProj, const mat4& worldTransform)
{
	vec4 view4 = ViewProj * vec4(worldTransform.el_2D[0][3], worldTransform.el_2D[1][3], worldTransform.el_2D[2][3], 1.0f);
	vec3 view(view4);
	return view.Lenght();
}

vec3 Line3D::projectPoint(vec3 &worldPos)
{
	vec3 AP = worldPos - origin;
	vec3 AB = direction;
	return origin + direction * AP.Dot(AB);
}
