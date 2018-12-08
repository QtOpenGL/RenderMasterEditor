#ifndef COMMON_H
#define COMMON_H

#include <QDebug>
#include <QString>
#include <VectorMath.h>

QString vec3ToString(const vec3& v)
{
	return QString('{') +
			QString::number(v.x) + QString(', ') +
			QString::number(v.y) + QString(', ') +
			QString::number(v.z) + QString('}');
}

float clamp(float f)
{
	return min(1.0f, max(0.0f, f));
}

template<typename T>
T lerp(const T& l, const T& r, float v)
{
	v = clamp(v);
	return l * (1.0f - v) + r * v;
}

#endif // COMMON_H
