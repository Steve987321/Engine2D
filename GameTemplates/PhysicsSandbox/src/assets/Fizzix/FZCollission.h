#pragma once

#include "engine/Types.h"

namespace fz
{
    class Polygon;

    bool LineLineIntersection(const Toad::Vec2f& p1, const Toad::Vec2f& p2, const Toad::Vec2f& q1, const Toad::Vec2f& q2, Toad::Vec2f& intersection);
    size_t ClipPolygon(const Polygon& a, const Polygon& b, Toad::Vec2f& contact);
    void ProjectPolygon(const Polygon& p, const Toad::Vec2f& axis, float& min, float& max);
    bool SAT(const Polygon& a, const Polygon& b, Toad::Vec2f& normal, float& overlap, Toad::Vec2f& contact, size_t& contact_count);
}
