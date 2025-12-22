#include "framework/Framework.h"
#include "FZRigidbody.h"

namespace fz
{
    void Rigidbody::Update(float dt)
    {
        // integrate velocities 
        center += velocity * velocity_damping * dt;
        angular_velocity *= angular_damping;

        center += center_correction;
        center_correction = Toad::Vec2f{0, 0};

        // check if rigidbody can sleep 
        time += dt;
        if (time > sleep_check_delay)
        {
            time = 0;
            Toad::Vec2f center_diff = center - center_prev;
            float abs_vel = abs(center_diff.x + center_diff.y);
            if (abs_vel <= 0.01f && angular_velocity <= 0.01f)
                is_sleeping = true;

            center_prev = center;
        }
    }
}
