#include "framework/Framework.h"
#include "FZRigidbody.h"
#include "FZPolygon.h"
#include "FZMath.h"

#include "FZWorld.h"

namespace fz
{
    std::array<Toad::Vec2f, 6> CreateSquare(float size_x, float size_y)
    {
        return CreateSquare({0, 0}, {size_x, size_y});
    }

    std::array<Toad::Vec2f, 6> CreateSquare(const Toad::Vec2f& start, const Toad::Vec2f& end)
    {
        return std::array<Toad::Vec2f, 6>({{start.x, end.y}, {end.x, end.y}, {end.x, start.y}, {start.x, end.y}, {end.x, start.y},  {start.x, start.y}});
    }

    Polygon::Polygon(const std::vector<Toad::Vec2f>& points)
        : vertices(points)
    {
        UpdateAABB();
        UpdateNormals();
        UpdateCentroid();
        UpdateMomentOfInertia(rb.center); // requires the updated centroid so this will be called last
    }

    void Polygon::UpdateNormals()
    {
        normals.clear();
        size_t n = vertices.size(); 
        for (size_t i = 0; i < n; i++)
        {
            Toad::Vec2f edge = vertices[(i + 1) % n] - vertices[i];
            Toad::Vec2f normal(-edge.y, edge.x); 
            normals.push_back(normal.Normalize());
        }
    }

    void Polygon::Translate(const Toad::Vec2f& offset)
    {
        for (Toad::Vec2f& v : vertices)
            v += offset;
        rb.center = rb.center + offset;

        UpdateAABB();
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
                v = &world->springs[i].end_rel;
            else 
                v = &world->springs[i].start_rel;

            float x = v->x;
            v->x = x * c - v->y * s;
            v->y = x * s + v->y * c;
        }

        for (auto& i : attached_thruster_points)
        {
            Toad::Vec2f* v = &world->thrusters[i].attached_rel_pos;

            float x = v->x;
            v->x = x * c - v->y * s;
            v->y = x * s + v->y * c;
        }

        UpdateNormals();
        UpdateAABB();
    }

    // https://lexrent.eu/wp-content/uploads/torza/artikel_groep_sub_2_docs/BYZ_3_Polygon-Area-and-Centroid.pdf
    void Polygon::UpdateCentroid()
    {
        float area = 0;
        float cx = 0, cy = 0;
        size_t vertices_count = vertices.size();

        for (size_t i = 0; i < vertices_count; i++)
        {
            size_t j = (i + 1) % vertices_count;
            float c = cross(vertices[i], vertices[j]);
            area += c;
            cx += (vertices[i].x + vertices[j].x) * c;
            cy += (vertices[i].y + vertices[j].y) * c;
        }

        area /= 2.f;

        if (std::abs(area) < FLT_EPSILON)
            return;

        area = 1 / (6 * area);
        rb.center = Toad::Vec2f(area * cx, area * cy);
    }

    void Polygon::UpdateMomentOfInertia(const Toad::Vec2f& center)
    {
        float inertia = 0.0f;
        size_t vertices_count = vertices.size();

        for (size_t i = 0; i < vertices_count; i++) 
        {
            size_t j = (i + 1) % vertices_count;
            Toad::Vec2f v1 = vertices[i];
            Toad::Vec2f v2 = vertices[j];
            Toad::Vec2f triangle_center = (v1 + v2 + center) / 3.f;

            float r1 = length(v1 - center);
            float r2 = length(v2 - center);
            inertia += (r1 * r1 + r2 * r2 + r1 * r2);
        }

        rb.moment_of_inertia = inertia;
    }

    void Polygon::UpdateAABB()
    {
        aabb = OBBToAABBSlow(vertices);
    }

    bool Polygon::ContainsPoint(const Toad::Vec2f& point)
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