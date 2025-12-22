#include "FZMath.h"

namespace fz
{
    float length(const Toad::Vec2f& v)
    {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }

    Toad::Vec2f normalize(const Toad::Vec2f& v)
    {
        float len = length(v);
        if (len == 0) 
            return {};
        return {v.x / len, v.y / len};
    }

    float cross(const Toad::Vec2f& a, const Toad::Vec2f& b)
    {
        return a.x * b.y - a.y * b.x;
    }

    Toad::Vec2f cross(const Toad::Vec2f& v, float a)
    {
        return Toad::Vec2f(a * v.y, -a * v.x);
    }

    Toad::Vec2f cross(float a, const Toad::Vec2f& v)
    {
        return Toad::Vec2f(-a * v.y, a * v.x);
    }

    float dot(const Toad::Vec2f& a, const Toad::Vec2f& b)
    {
        return a.x * b.x + a.y * b.y;
    }

    float dist(const Toad::Vec2f& a, const Toad::Vec2f& b)
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    AABB OBBToAABBSlow(const std::vector<Toad::Vec2f>& vertices)
    {
        AABB res;
        res.min.x = FLT_MAX;
        res.min.y = FLT_MAX;
        res.max.x = std::numeric_limits<float>::lowest();
        res.max.y = std::numeric_limits<float>::lowest();

        for(const Toad::Vec2f& v : vertices)
        {
            if (v.x < res.min.x)
                res.min.x = v.x;
            if (v.x > res.max.x)
                res.max.x = v.x;

            if (v.y < res.min.y)
                res.min.y = v.y;
            if (v.y > res.max.y)
                res.max.y = v.y;
        }

        return res;
    }
}