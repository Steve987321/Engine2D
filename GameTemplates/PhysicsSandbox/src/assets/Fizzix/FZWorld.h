#pragma once

#include "FZPolygon.h"
#include "FZRigidbody.h"
#include "FZSpring.h"
#include "FZThruster.h"

namespace fz
{
    class World
    {
    public:
        World();
        
        void Update(float dt);

        size_t AddPolygon(fz::Polygon& polygon);
        void RemovePolygon(fz::Polygon& polygon);

        // spawns spring with min_len set to third of the spring distance (target_len)
        fz::Spring& AddSpring(Polygon& start, Polygon& end, const Toad::Vec2f& rel_start, const Toad::Vec2f& rel_end);
        size_t AddThruster(Polygon& polygon, const Toad::Vec2f& rel);
        size_t AddThruster(size_t p_id, const Toad::Vec2f& rel);

        void UpdatePolygonVertices(uint32_t polygon_index);
        void ResetRenderingState();

        void ApplyDirectionalForce(const Toad::Vec2f& pos, const Toad::Vec2f& dir, float width = 1.f, float force = 100.f);
        void ApplyForceSphere(const Toad::Vec2f& pos, float radius = 10.f, float force = 100.f);
        
        Toad::Vec2f gravity{0.f, 9.8f};
        std::vector<Polygon> polygons {};
        std::vector<Thruster> thrusters {};
        std::vector<Spring> springs;

        Toad::DrawingCanvas dc;
    };
}
