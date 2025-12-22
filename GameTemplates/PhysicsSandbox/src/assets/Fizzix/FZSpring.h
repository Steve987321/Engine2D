#pragma once

#include "engine/Types.h"

namespace fz
{
    class Rigidbody; 
    
    // #todo: this is not very stable
    class Spring
    {
    public:
        void Update(float dt);

        float stiffness = 0.1f; 

        float min_len = 0.f;
        float target_len = 0.f;
        float compression_damping = .95f;
        float rebound_damping = .95f;
        float rotation_force_factor = 20.f;

        // don't edit, just for testing 
        float len = 0;

        // relative to center of rbs
        Toad::Vec2f start_rel;
        Toad::Vec2f end_rel;

        Rigidbody* start_rb = nullptr;
        Rigidbody* end_rb = nullptr;

    private:
        // update start_rel and end_rel posititions using the rigidbodies angular velocities
        void UpdateRotation(float dt);
    };
}