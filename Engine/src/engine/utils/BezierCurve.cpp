#include "pch.h"
#include "BezierCurve.h"

namespace Toad
{
	
	Vec2f QuadraticBezier(const Vec2f& a, const Vec2f& b, const Vec2f& control_point, float t)
	{
		return Vec2f(
			(1 - t) * (1 - t) * a.x + (1 - t) * 2 * t * control_point.x + t * t * b.x,
			(1 - t) * (1 - t) * a.y + (1 - t) * 2 * t * control_point.y + t * t * b.y
		);
	}

	//Vec2f MultiBezier(const Vec2f& a, const Vec2f& b, const Vec2f control_points[], uint32_t control_points_len, float t)
	//{
	//	if (control_points_len <= 1)
	//	{
	//		assert(control_points && "control_points argument is null");
	//		return QuadraticBezier(a, b, *control_points, t);
	//	}

	//	for (uint32_t i = 0, j = 1; j < control_points_len - 1; i++, j++)
	//	{
	//		Vec2f mid_point = (control_points[i] + control_points[j]) / 2;
	//	}
	//}

}