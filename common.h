#ifndef COMMON_H
#define COMMON_H

#include <QDebug>
#include <QString>
#include <VectorMath.h>

QString vec3ToString(const vec3& v);
float clamp(float f);

template<typename T>
T lerp(const T& l, const T& r, float v)
{
	v = clamp(v);
	return l * (1.0f - v) + r * v;
}

#endif // COMMON_H
