#include "ManipulatorUtils.h"

bool LineIntersectPlane(vec3& intersection, const Plane& plane, const Line3D& line)
{
	// assuming vectors are all normalized
	vec3 R = line.direction.Normalized();
	vec3 N = plane.normal.Normalized();

	float denom = N.Dot(R);
	if (std::abs(denom) > 1e-6) {
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
