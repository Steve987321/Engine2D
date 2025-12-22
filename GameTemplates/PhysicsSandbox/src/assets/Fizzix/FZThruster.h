#pragma once 

#include "engine/Types.h"

namespace fz
{
    class Thruster
    {
    public:
        void Update(float dt);

        size_t attached_polygon;
        size_t id;
        Toad::Vec2f attached_rel_pos;

        void SetDirection(float degrees);
        float direction_deg = 0;
        Toad::Vec2f direction {1, 0};

        float power = 0.f; 
        float max_power = 1000.f;

        // thruster blast will cause wind effect on objects below 
        bool thrust_affects_other_objects = false;

        fz::World* world = nullptr;
    private:
        float direction_rad = 0;
    };
}