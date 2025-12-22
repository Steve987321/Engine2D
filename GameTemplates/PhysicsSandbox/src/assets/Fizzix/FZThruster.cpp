#include "framework/Framework.h"
#include "engine/utils/Helpers.h"

#include "FZPolygon.h"
#include "FZWorld.h"
#include "FZMath.h"
#include "FZThruster.h"

namespace fz
{

void Thruster::Update(float dt)
{
    if (Toad::Input::IsKeyDown(Toad::Keyboard::Key::Space))
        power = max_power;
    else{
        power = 0;
        return;
    }

    fz::Polygon& p = world->polygons[attached_polygon];

    p.rb.is_sleeping = false;

    Toad::Vec2f start_pos = p.rb.center + attached_rel_pos;
    Toad::Vec2f end_pos = start_pos + direction * power;
    Toad::Vec2f dir_norm = (end_pos - start_pos).Normalize();

    float d = dist(start_pos, end_pos);

    Toad::Vec2f rb_force = dir_norm * d * p.rb.inv_mass;
    p.rb.velocity += rb_force;

    // apply angular velocity 
    float torque = cross(attached_rel_pos, rb_force);

    p.rb.angular_velocity += torque / p.rb.moment_of_inertia;
}

void Thruster::SetDirection(float degrees)
{
    direction_deg = degrees;
    direction_rad = Toad::DegToRad(degrees);  
    direction = Toad::Vec2f{cos(direction_rad), sin(direction_rad)};
}

}