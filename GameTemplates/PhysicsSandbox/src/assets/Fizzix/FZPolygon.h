#pragma once

#include "FZRigidbody.h"
#include "FZTypes.h"

namespace fz
{
    std::array<Toad::Vec2f, 6> CreateSquare(float size_x, float size_y);
    std::array<Toad::Vec2f, 6> CreateSquare(const Toad::Vec2f& start, const Toad::Vec2f& end);
    
    class World;
    class Spring;
    
    class Polygon
    {
    public:
        // Create a polygon based on vertex array, must be convex
        Polygon(const std::vector<Toad::Vec2f>& points);

        size_t id = 0;
        std::vector<Toad::Vec2f> vertices;
        std::vector<Toad::Vec2f> normals;
        AABB aabb;

        // #TODO: sweep and prune breaks this 
        // springs attached to this polygon that need to be updated, hold spring index and whether its end(true) or start(false)
        std::vector<std::pair<size_t, bool>> attached_spring_points;
        std::vector<size_t> attached_thruster_points;

        Rigidbody rb;
        World* world = nullptr;

        void UpdateNormals();
        void UpdateCentroid();
        void UpdateMomentOfInertia(const Toad::Vec2f& center);
        void UpdateAABB();

        void Translate(const Toad::Vec2f& offset);
        void Rotate(float angle);

        // check whether the point is in the polygon
        bool ContainsPoint(const Toad::Vec2f& point);
    };
}
