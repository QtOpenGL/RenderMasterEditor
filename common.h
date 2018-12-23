#ifndef COMMON_H
#define COMMON_H

#undef min
#undef max

#include <algorithm>
#include <QDebug>
#include <QString>
#include <VectorMath.h>

QString vec3ToString(const vec3& v);
QString quatToString(const quat& v);
QString vec2ToString(const vec2& v);
float clamp(float f);

template<typename T>
T lerp(const T& l, const T& r, float v)
{
	v = clamp(v);
	return l * (1.0f - v) + r * v;
}

inline float clamp(float n, float lower, float upper)
{
    return std::max(lower, std::min(n, upper));
}

struct Spherical
{
	float r;
	float theta;
	float phi;
};

QString sphericalToString(const Spherical& v);

Spherical ToSpherical(const vec3& pos);
vec3 ToCartesian(const Spherical& pos);

void lookAtCamera(mat4& Result, const vec3 &eye, const vec3 &center);


#endif // COMMON_H
