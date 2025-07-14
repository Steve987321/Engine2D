#pragma once

#include "engine/Types.h"

namespace fz
{
    inline float length(const Toad::Vec2f& v)
    {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }

    inline Toad::Vec2f normalize(const Toad::Vec2f& v)
    {
        float len = length(v);
        if (len == 0) 
            return {};
        return {v.x / len, v.y / len};
    }

    inline float cross(const Toad::Vec2f& a, const Toad::Vec2f& b)
    {
        return a.x * b.y - a.y * b.x;
    }

    inline Toad::Vec2f cross(const Toad::Vec2f& v, float a)
    {
        return Toad::Vec2f( a * v.y, -a * v.x );
    }

    inline Toad::Vec2f cross(float a, const Toad::Vec2f& v)
    {
        return Toad::Vec2f( -a * v.y, a * v.x );
    }

    inline float dot(const Toad::Vec2f& a, const Toad::Vec2f& b)
    {
        return a.x * b.x + a.y * b.y;
    }

    inline float dist(const Toad::Vec2f& a, const Toad::Vec2f& b)
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

}
