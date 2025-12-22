#include "framework/Framework.h"
#include "FZMisc.h"

#include "FZPolygon.h"
#include "FZCollission.h"
#include "FZMath.h"

namespace fz
{
    using namespace Toad;

    static float SignedDistance(const Vec2f& p, const Vec2f& a, const Vec2f& b) 
    {
        Vec2f ab = b - a;
        Vec2f ap = p - a;
        return cross(ab, ap);
    }

    std::pair<Polygon, Polygon> SplitPolygon(const Polygon& p, const Vec2f& a, const Vec2f& b)
    {
        std::vector<Vec2f> front;
        std::vector<Vec2f> back;

        const std::vector<Vec2f>& verts = p.vertices;
        int n = verts.size();

        for (int i = 0; i < n; i++)
        {
            Vec2f curr = verts[i];
            Vec2f next = verts[(i + 1) % n];

            float currDist = SignedDistance(curr, a, b);
            float nextDist = SignedDistance(next, a, b);

            if (currDist >= 0) front.push_back(curr);
            if (currDist <= 0) back.push_back(curr);

            // Check for edge crossing
            if ((currDist > 0 && nextDist < 0) || (currDist < 0 && nextDist > 0))
             {
                Vec2f intersection;
                if (LineLineIntersection(curr, next, a, b, intersection))
                {
                    front.push_back(intersection);
                    back.push_back(intersection);
                }
            }
        }

        return {Polygon(front), Polygon(back)};
    }
}