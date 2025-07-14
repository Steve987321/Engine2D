#include "framework/Framework.h"
#include "FZRigidbody.h"
#include "FZPolygon.h"
#include "FZMath.h"

#include "FZSim.h"

namespace fz
{
    std::array<Toad::Vec2f, 6> CreateSquare(float size_x, float size_y)
    {
        return std::array<Toad::Vec2f, 6>({{0, size_y}, {size_x, size_y}, {size_x, 0}, {0, size_y}, {size_x, 0},  {0, 0}});
    }

    std::array<Toad::Vec2f, 6> CreateSquare(const Toad::Vec2f& start, const Toad::Vec2f& end)
    {
        return std::array<Toad::Vec2f, 6>({{start.x, end.y}, {end.x, end.y}, {end.x, start.y}, {start.x, end.y}, {end.x, start.y},  {start.x, start.y}});
    }

    Polygon::Polygon(const std::vector<Toad::Vec2f>& points)
        : vertices(points)
    {
        UpdateNormals();
        UpdateCentroid();

        float inertia = 0.0f;
        size_t vertices_count = vertices.size();

        for (size_t i = 0; i < vertices_count; i++) 
        {
            Toad::Vec2f v1 = vertices[i];
            Toad::Vec2f v2 = vertices[(i + 1) % vertices_count];
            
            float r1 = length(v1 - rb.center);
            float r2 = length(v2 - rb.center);
            inertia += (r1 * r1 + r2 * r2 + r1 * r2);
        }
        rb.moment_of_inertia = inertia;
    }

    void Polygon::UpdateNormals()
    {
        normals.clear();
        int n = vertices.size();
        for (int i = 0; i < n; i++)
        {
            Toad::Vec2f edge = vertices[(i + 1) % n] - vertices[i];
            Toad::Vec2f normal(-edge.y, edge.x); 
            normals.push_back(normalize(normal));
        }
    }

    void Polygon::Translate(const Toad::Vec2f& offset)
    {
        for (Toad::Vec2f& v : vertices)
            v += offset;
        rb.center = rb.center + offset;
    }

    void Polygon::Rotate(float angle)
    {
        float c = std::cos(angle);
        float s = std::sin(angle);

        for (Toad::Vec2f& v : vertices)
        {
            float x = v.x - rb.center.x;
            float y = v.y - rb.center.y;
            v.x = rb.center.x + x * c - y * s;
            v.y = rb.center.y + x * s + y * c;
        }

        for (auto& [i, is_end]: attached_spring_points)
        {
            Toad::Vec2f* v = nullptr;
            if (is_end)
                v = &sim->springs[i].end_rel;
            else 
                v = &sim->springs[i].start_rel;

            float x = v->x;
            v->x = x * c - v->y * s;
            v->y = x * s + v->y * c;
        }

        UpdateNormals();
    }

    void Polygon::UpdateCentroid()
    {
        float area = 0;
        float cx = 0, cy = 0;
        int vertices_count = vertices.size();

        for (int i = 0; i < vertices_count; i++)
        {
            int j = (i + 1) % vertices_count;
            float c = cross(vertices[i], vertices[j]);
            area += c;
            cx += (vertices[i].x + vertices[j].x) * c;
            cy += (vertices[i].y + vertices[j].y) * c;
        }

        area /= 2.f;

        if (std::abs(area) < FLT_EPSILON)
            return;

        rb.center = Toad::Vec2f(cx / (6 * area), cy / (6 * area));
    }

    bool Polygon::ContainsPoint(const Toad::Vec2f &point)
    {
        int vertices_count = vertices.size();
        bool inside = false;
        for (int i = 0, j = vertices_count - 1; i < vertices_count; j = i++) 
        {
            if ((vertices[i].y > point.y) != (vertices[j].y > point.y))
            {
                double x_intersect = (vertices[j].x - vertices[i].x) * (point.y - vertices[i].y) /
                                     (vertices[j].y - vertices[i].y) + vertices[i].x;
                if (point.x < x_intersect)
                    inside = !inside;
            }
        }
        return inside;
    }
}