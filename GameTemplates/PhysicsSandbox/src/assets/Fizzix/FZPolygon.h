#pragma once

#include "FZRigidbody.h"

namespace fz
{
    std::array<Toad::Vec2f, 6> CreateSquare(float size_x, float size_y);
    std::array<Toad::Vec2f, 6> CreateSquare(const Toad::Vec2f& start, const Toad::Vec2f& end);
    
    class Sim;
    class Spring;
    
    class Polygon
    {
    public:
        std::vector<Toad::Vec2f> vertices;
        std::vector<Toad::Vec2f> normals;

        // springs attached to this polygon that need to be updated, hold spring index and whether its end
        std::vector<std::pair<size_t, bool>> attached_spring_points;

        Rigidbody rb;
        Sim* sim = nullptr;

        // Create a polygon based on vertex array, must be convex
        Polygon(const std::vector<Toad::Vec2f>& points);

        void UpdateNormals();
        void UpdateCentroid();

        void Translate(const Toad::Vec2f& offset);
        void Rotate(float angle);

        // check whether the point is in the polygon
        bool ContainsPoint(const Toad::Vec2f& point);
    };
}
