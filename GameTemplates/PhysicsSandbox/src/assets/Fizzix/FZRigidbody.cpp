#include "framework/Framework.h"
#include "FZRigidbody.h"

namespace fz
{
    void Rigidbody::Update(float dt)
    {
        Toad::Vec2f damping = velocity_damping;
        float rot_damp = angular_damping;
        if (resting)
        {
            damping.x = std::lerp(damping.x, 0.1f, dt * 10);
            damping.y = std::lerp(damping.y, 0.1f, dt * 10);
            rot_damp =std::lerp(rot_damp, 0.1f, dt * 10);
            
            // check unrest 
            if (velocity.Length() > 0.5f || angular_velocity > 0.1f)
                resting = false;
        }
        
        time += dt;
        if (time > 0.02f)
        {
            if (velocities.size() > 20)
                velocities.pop_back();

            HistoryData d;
            d.velocity = velocity;
            d.angular_velocity = angular_velocity;
            velocities.push_front(d);
            time = 0;
        }

        velocity_average = Toad::Vec2f{0, 0};
        angular_velocity_average = 0;
        for (const HistoryData& v : velocities)
        {
            velocity_average += v.velocity;
            angular_velocity_average += v.angular_velocity;
        }
        velocity_average /= velocities.size();
        angular_velocity_average /= velocities.size();

        center += velocity * damping * dt;
        angular_velocity *= rot_damp;

        center += center_correction;
        center_correction = Toad::Vec2f{0, 0};
    }
}
