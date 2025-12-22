#pragma once

#include "framework/Framework.h"
#include "engine/Types.h"
#include "FZTypes.h"

// #TODO: Most of these have been implemented in the Vec2 type 

namespace fz
{
    float length(const Toad::Vec2f& v);

    Toad::Vec2f normalize(const Toad::Vec2f& v);

    float cross(const Toad::Vec2f& a, const Toad::Vec2f& b);
    Toad::Vec2f cross(const Toad::Vec2f& v, float a);
    Toad::Vec2f cross(float a, const Toad::Vec2f& v);

    float dot(const Toad::Vec2f& a, const Toad::Vec2f& b);

    float dist(const Toad::Vec2f& a, const Toad::Vec2f& b);

    AABB OBBToAABBSlow(const std::vector<Toad::Vec2f>& vertices);
}
