#include "framework/Framework.h"
#include "FZMath.h"
#include "FZRigidbody.h"
#include "FZSpring.h"

namespace fz
{
    void Spring::Update(float dt)
    {
        if (!start_rb || !end_rb)
            return;

        // UpdateRotation(dt);
        start_rb->is_sleeping = false;
        end_rb->is_sleeping = false;

        Toad::Vec2f start_pos = start_rb->center + start_rel;
        Toad::Vec2f end_pos = end_rb->center + end_rel;
        Toad::Vec2f dir_norm = (end_pos - start_pos).Normalize();

        if (target_len != 0)
        {
            len = dist(start_pos, end_pos); // * 0.9f;
            float correct = target_len - len;
            Toad::Vec2f start_pos_a = start_rb->center + start_rel;
            Toad::Vec2f end_pos_a = ((end_pos + Toad::Vec2f(-dir_norm * correct / 2.f)) + start_pos_a + dir_norm * correct / 2.f) / 2.f;
            float d = dist(start_pos_a, end_pos_a); // * 0.9f;
            // Toad::DrawingCanvas::DrawArrow(start_pos, dir_norm * 2.f, 1.f, Toad::Color::Blue);
            // Toad::DrawingCanvas::DrawText(end_pos_a, std::to_string(d), 4.f);

            float stiffness_after_damping = stiffness;
            if (correct > 0)
            {
                dir_norm *= -1;
                stiffness_after_damping *= compression_damping;
            }
            else 
                stiffness_after_damping *= rebound_damping;
            
            Toad::Color sitffness_as_col(0, 0, (uint8_t)((stiffness_after_damping / std::max(stiffness, 1.f)) * 255.f), 255);
            Toad::DrawingCanvas::DrawArrow(end_pos_a, {0, 5.f}, 1.f, sitffness_as_col);

            Toad::Vec2f start_rb_force = dir_norm * d * stiffness_after_damping * start_rb->inv_mass;
            Toad::Vec2f end_rb_force = dir_norm * d * stiffness_after_damping * end_rb->inv_mass;

            start_rb->velocity += start_rb_force;
            end_rb->velocity -= end_rb_force;

            // apply angular velocity 
            float torque_start = cross(start_rel, start_rb_force * rotation_force_factor);
            float torque_end = cross(end_rel, end_rb_force * rotation_force_factor);
            // Toad::DrawingCanvas::DrawText(end_pos_a, std::to_string(torque_start), 10);

            start_rb->angular_velocity += torque_start / start_rb->moment_of_inertia;
            end_rb->angular_velocity -= torque_end / end_rb->moment_of_inertia;
            
            return; 
        }
        
        float d = dist(start_pos, end_pos);

        if (d < min_len)
        {
            float correct = min_len - d;
            start_rb->center_correction -= dir_norm * correct / 2.f;
            end_rb->center_correction += dir_norm * correct / 2.f;
            d = min_len;
        }

        start_rb->velocity += dir_norm * d * stiffness * start_rb->inv_mass;
        end_rb->velocity -= dir_norm * d * stiffness * end_rb->inv_mass;
    }

    void Spring::UpdateRotation(float dt)
    {
        float start_angle = start_rb->angular_velocity * dt;
        float end_angle = end_rb->angular_velocity * dt;

        float start_cos = cos(start_angle);
        float start_sin = sin(start_angle);
        start_rel.x = start_rel.x * start_cos - start_rel.y * start_sin;
        start_rel.y = start_rel.x * start_sin + start_rel.y * start_cos;

        float end_cos = cos(end_angle);
        float end_sin = sin(end_angle);
        end_rel.x = end_rel.x * end_cos - end_rel.y * end_sin;
        end_rel.y = end_rel.x * end_sin + end_rel.y * end_cos;
    }
}