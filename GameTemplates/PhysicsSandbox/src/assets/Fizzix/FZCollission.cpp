#include "framework/Framework.h"

#include "FZCollission.h"
#include "FZPolygon.h"
#include "FZMath.h"

namespace fz
{
    using namespace Toad;
    
    bool LineLineIntersection(const Vec2f& p1, const Vec2f& p2, const Vec2f& q1, const Vec2f& q2, Vec2f& intersection)
    {
        Vec2f r = p2 - p1;
        Vec2f s = q2 - q1;
        
        float rxs = r.Cross(s);
        float qpxr = (q1 - p1).Cross(r);

        // check if colinear 
        if (fabs(rxs) < FLT_EPSILON) 
            return false;

        float t = (q1 - p1).Cross(s) / rxs;
        float u = (q1 - p1).Cross(r) / rxs;

        // intersects when t and u are between 0 and 1
        if (t >= 0 && t <= 1 && u >= 0 && u <= 1)
        {
            intersection = p1 + r * t;
            return true;
        }

        return false;
    }

    // find contacts between all edges of a and b 
    size_t ClipPolygon(const Polygon& a, const Polygon& b, Vec2f& contact)
    {
        size_t intersection_count = 0;
        for (size_t i = 0; i < a.vertices.size(); i++)
        {
            size_t j = (i + 1) % a.vertices.size();
            
            Vec2f p1 = a.vertices[i];
            Vec2f p2 = a.vertices[j];

            for (size_t k = 0; k < b.vertices.size(); k++)
            {
                size_t l = (k + 1) % b.vertices.size();

                Vec2f q1 = b.vertices[k];
                Vec2f q2 = b.vertices[l];

                Vec2f intersection;
                if (LineLineIntersection(p1, p2, q1, q2, intersection))
                {
                    contact += intersection;
                    intersection_count++;
                }
            }
        }

        contact /= intersection_count;

        return intersection_count;
    }

    void ProjectPolygon(const Polygon& p, const Vec2f& axis, float& min, float& max)
    {
        min = FLT_MAX;
        max = -FLT_MAX; 
        
        for (const Vec2f& v : p.vertices)
        {
            float proj = dot(v, axis);

            if (proj < min)
                min = proj;

            if (proj > max)
                max = proj;
        }
    }

    bool SAT(const Polygon& a, const Polygon& b, Vec2f& normal, float& overlap, Vec2f& contact, size_t& contact_count)
    {
        float min_overlap = FLT_MAX;
        Vec2f best_normal;
        
        for (const Vec2f& axis : a.normals)
        {
            float min_a, max_a, min_b, max_b;
            ProjectPolygon(a, axis, min_a, max_a);
            ProjectPolygon(b, axis, min_b, max_b);

            if (max_a < min_b || max_b < min_a)
                return false;

            float ab_overlap = std::min(max_a - min_b, max_b - min_a);
            if (ab_overlap < min_overlap)
            {
                min_overlap = ab_overlap;
                best_normal = axis;
            }
        }
        
        for (const Vec2f& axis : b.normals)
        {
            float min_a, max_a, min_b, max_b;
            ProjectPolygon(a, axis, min_a, max_a);
            ProjectPolygon(b, axis, min_b, max_b);

            if (max_a < min_b || max_b < min_a)
                return false;

            float ab_overlap = std::min(max_a - min_b, max_b - min_a);
            if (ab_overlap < min_overlap)
            {
                min_overlap = ab_overlap;
                best_normal = axis;
            }
        }
        
        // check facing direction and flip if needed
        Vec2f ab = b.rb.center - a.rb.center;
        if (ab.Dot(best_normal) < 0)
        {
            best_normal = -best_normal;
        }
    
        normal = best_normal;
        overlap = min_overlap;
    
        // get contact point 
        contact_count = ClipPolygon(a, b, contact);
        
        if (contact_count == 0)
        {
            // fallback
            contact = (a.rb.center + b.rb.center) * 0.5f;
        }
    
        return true;
    }
}
