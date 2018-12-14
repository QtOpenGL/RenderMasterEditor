#include "ManipulatorUtils.h"

bool LineIntersectPlane(vec3& intersection, const Plane& plane, const Line3D& line)
{
	// assuming vectors are all normalized
	float denom = plane.normal.Dot(line.direction);
	if (denom > 1e-6) {
		vec3 p0l0 = plane.origin - line.origin;
		float t = p0l0.Dot(plane.normal) / denom;
		if (t >= 0)
		{
			intersection = line.origin + line.direction * t;
			return true;
		}
		return false;
	}

	return false;
}
