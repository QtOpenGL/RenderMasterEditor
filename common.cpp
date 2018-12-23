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

Spherical ToSpherical(const vec3& pos)
{
	float r = sqrt(pos.Dot(pos));
	float theta = acos(pos.z / r);
	float phi = atan2(pos.y, pos.x);

	return Spherical{r, theta, phi};
}

vec3 ToCartesian(const Spherical &pos)
{
	return vec3(
				pos.r * sin(pos.theta) * cos(pos.phi),
				pos.r * sin(pos.theta) * sin(pos.phi),
				pos.r * cos(pos.theta));
}


QString sphericalToString(const Spherical &v)
{
	return QString("{r: ") +
			QString::number(v.r) + QString(", theta: ") +
			QString::number(v.theta) + QString(", phi: ") +
			QString::number(v.phi) + QString('}');
}

void lookAtCamera(mat4& Result, const vec3 &eye, const vec3 &center)
{
	Result = Matrix4x4(1.0f);
	Vector3 Z = (eye - center).Normalize();
	Vector3 X = Vector3(0.0f, 0.0f, 1.0f).Cross(Z).Normalize();
	Vector3 Y(Z.Cross(X));
	Y.Normalize();
	Result.el_2D[0][0] = X.x;
	Result.el_2D[0][1] = X.y;
	Result.el_2D[0][2] = X.z;
	Result.el_2D[1][0] = Y.x;
	Result.el_2D[1][1] = Y.y;
	Result.el_2D[1][2] = Y.z;
	Result.el_2D[2][0] = Z.x;
	Result.el_2D[2][1] = Z.y;
	Result.el_2D[2][2] = Z.z;
	Result.el_2D[0][3] = -X.Dot(eye);
	Result.el_2D[1][3] = -Y.Dot(eye);
	Result.el_2D[2][3] = -Z.Dot(eye);
}

QString quatToString(const quat &q)
{
	return QString("{x: ") +
			QString::number(q.x) + QString(", y: ") +
			QString::number(q.y) + QString(", z: ") +
			QString::number(q.z) + QString(", w: ") +
			QString::number(q.w) + QString('}');
}
