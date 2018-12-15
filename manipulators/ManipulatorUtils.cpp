#include "ManipulatorUtils.h"
#include <algorithm>

bool LineIntersectPlane(vec3& intersection, const Plane& plane, const Line3D& line)
{
	vec3 R = line.direction.Normalized();
	vec3 N = plane.normal.Normalized();

	float denom = N.Dot(R);
    if (denom > 1e-6) {
		vec3 p0l0 = plane.origin - line.origin;
        float t = p0l0.Dot(N) / denom;
		if (t >= 0)
		{
			intersection = line.origin + R * t;
			return true;
		}
		return false;
	}

	return false;
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

    //qDebug() << "ray dir: " << vec3ToString(dir) << "proprortion: " << vec3ToString(vec3(origin.x / dir.x, origin.y / dir.y, origin.z / dir.z)) <<
	//qDebug() << "mousePos: " << mousePos.x << " " << mousePos.y;

    return Line3D(dir, origin);
}

vec2 WorldToNdc(const vec3& pos, const mat4& ViewProj)
{
    vec4 screenPos = ViewProj * vec4(pos);
    screenPos /= screenPos.w;
    return vec2(screenPos.x, screenPos.y);
}

float SegmentPointDistance(const vec2& p0, const vec2& p1, const vec2& point)
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
