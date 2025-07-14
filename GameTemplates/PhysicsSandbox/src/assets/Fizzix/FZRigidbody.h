#pragma once

namespace fz
{
    struct HistoryData
    {
        Toad::Vec2f velocity;
        float angular_velocity;
    };

    class Rigidbody
    {
    public:
        void Update(float dt);

        bool resting = false; 
        
        float angular_damping = 0.95f;
        float angular_velocity = 0;
        float moment_of_inertia = 0.1f;
        float restitution = 0.2f;
        float mass = 10.f;
        bool is_static = false;
        Toad::Vec2f velocity {0, 0}; 
        Toad::Vec2f velocity_damping = {1, 1};
        Toad::Vec2f center {0, 0};
        Toad::Vec2f center_correction {0, 0};

        Toad::Vec2f velocity_average {0, 0};
        float angular_velocity_average = 0;

        float friction = 0.f;

        float slide = 0.f;
    private:
        std::deque<HistoryData> velocities {};
        float time = 0;
    };
}
