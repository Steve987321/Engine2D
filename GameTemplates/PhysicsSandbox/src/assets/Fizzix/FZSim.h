#pragma once

#include "FZPolygon.h"
#include "FZRigidbody.h"
#include "FZSpring.h"

namespace fz
{
    class Sim
    {
    public:
        void Update(float dt);
        void Resolve(Rigidbody& a, Rigidbody& b, const Toad::Vec2f& contact, const Toad::Vec2f& normal, float penetration);

        fz::Polygon& AddPolygon(fz::Polygon& polygon);

        // spawns spring with min_len set to third of the spring distance (target_len)
        fz::Spring& AddSpring(Polygon& start, Polygon& end, const Toad::Vec2f& rel_start, const Toad::Vec2f rel_end);

        Toad::Vec2f gravity{0.f, 9.8f};
        std::vector<Polygon> polygons {};
        std::vector<Spring> springs;
    };
}
