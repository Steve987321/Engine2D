#pragma once

namespace fz
{
    class Rigidbody
    {
    public:
        void Update(float dt);

        float angular_damping = 0.99f;
        float angular_velocity = 0;
        float moment_of_inertia = 1000.f;
        float restitution = 0.2f;
        float inv_mass = 0.1f;
        bool is_static = false;
        bool is_sleeping = false;
        uint32_t sleeping_ticks = 0;

        Toad::Vec2f velocity {0, 0}; 
        Toad::Vec2f velocity_damping = {1, 1};
        Toad::Vec2f center {0, 0};
        Toad::Vec2f center_correction {0, 0};

        float friction = 0.f;
        float slide = 0.f;
        
        inline static float sleep_check_delay = 1.f; // seconds  
        inline static float time = 0.f; // time that gets reset after passing sleep_check_delay  
        Toad::Vec2f center_prev;     
    };
}
