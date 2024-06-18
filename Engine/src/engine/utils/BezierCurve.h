#pragma once

#include "EngineCore.h"
#include "engine/Types.h"

namespace Toad
{
	Vec2f QuadraticBezier(const Vec2f& a, const Vec2f& b, const Vec2f& control_point, float t);

	//Vec2f MultiBezier(const Vec2f& a, const Vec2f& b, const Vec2f control_points[], uint32_t control_points_len, float t);
}