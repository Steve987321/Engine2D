#pragma once

#include "engine/Types.h"

namespace fz
{
    class Polygon;
    class World;

    std::pair<Polygon, Polygon> SplitPolygon(const Polygon& p, const Toad::Vec2f& a, const Toad::Vec2f& b);
}
