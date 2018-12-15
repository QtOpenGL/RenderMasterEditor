#include "common.h"
#include <algorithm>

QString vec3ToString(const vec3& v)
{
    return QString('{') +
			QString::number(v.x) + QString(", ") +
			QString::number(v.y) + QString(", ") +
			QString::number(v.z) + QString('}');
}

QString vec2ToString(const vec2& v)
{
    return QString('{') +
			QString::number(v.x) + QString(", ") +
			QString::number(v.y) + QString('}');
}

float clamp(float f)
{
	return std::min(1.0f, std::max(0.0f, f));
}
