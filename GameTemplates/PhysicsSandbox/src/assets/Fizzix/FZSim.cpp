#include "framework/Framework.h"
#include "FZSim.h"

#include "FZMath.h"

#include "scripts/Sim.h"

namespace fz
{
    static std::vector<Toad::Vec2f> contacts;

    static bool LineLineIntersection(const Toad::Vec2f& p1, const Toad::Vec2f& p2, const Toad::Vec2f& q1, const Toad::Vec2f& q2, Toad::Vec2f& intersection)
    {
        Toad::Vec2f r = p2 - p1;
        Toad::Vec2f s = q2 - q1;
        
        float rxs = r.Cross(s);
        float qpxr = (q1 - p1).Cross(r);

        // check if colinear 
        if (fabs(rxs) < FLT_EPSILON) 
            return false;

        float t = (q1 - p1).Cross(s) / rxs;
        float u = (q1 - p1).Cross(r) / rxs;

        // intersects when t and u are between 0 and 1
        if (t >= 0 && t <= 1 && u >= 0 && u <= 1)
        {
            intersection = p1 + r * t;
            return true;
        }

        return false;
    }

    // find contacts between all edges of a and b 
    static void ClipPolygon(const Polygon& a, const Polygon& b, std::vector<Toad::Vec2f>& contacts)
    {
        for (size_t i = 0; i < a.vertices.size(); i++)
        {
            size_t j = (i + 1) % a.vertices.size();
            
            Toad::Vec2f p1 = a.vertices[i];
            Toad::Vec2f p2 = a.vertices[j];

            for (size_t k = 0; k < b.vertices.size(); k++)
            {
                size_t l = (k + 1) % b.vertices.size();

                Toad::Vec2f q1 = b.vertices[k];
                Toad::Vec2f q2 = b.vertices[l];

                Toad::Vec2f intersection;
                if (LineLineIntersection(p1, p2, q1, q2, intersection))
                    contacts.push_back(intersection);
            }
        }
    }

    static void ProjectPolygon(const Polygon& p, const Toad::Vec2f& axis, float& min, float& max)
    {
        min = FLT_MAX;
        max = -FLT_MAX; 
        
        for (const Toad::Vec2f& v : p.vertices)
        {
            float proj = dot(v, axis);

            if (proj < min)
                min = proj;

            if (proj > max)
                max = proj;
        }
    }

    static bool SAT(const Polygon& a, const Polygon& b, Toad::Vec2f& normal, float& penetration, Toad::Vec2f& contact)
    {
        float min_penetration = FLT_MAX;
        Toad::Vec2f best_normal;
        
        for (const auto& poly : {a, b})
        {
            for (const Toad::Vec2f& axis : poly.normals)
            {
                float min_a, max_a, min_b, max_b;
                ProjectPolygon(a, axis, min_a, max_a);
                ProjectPolygon(b, axis, min_b, max_b);
    
                if (max_a < min_b || max_b < min_a)
                    return false;
    
                float pen = std::min(max_a - min_b, max_b - min_a);
                if (pen < min_penetration)
                {
                    min_penetration = pen;
                    best_normal = axis;
                }
            }
        }
        
        // check facing direction and flip if needed
        Toad::Vec2f ab = b.rb.center - a.rb.center;
        if (ab.Dot(best_normal) < 0)
        {
            best_normal = -best_normal;
        }
    
        normal = best_normal;
        penetration = min_penetration;
    
        // get contact point 
        contacts.clear();
        ClipPolygon(a, b, contacts);
        ClipPolygon(b, a, contacts);
        
        if (!contacts.empty())
        {
            contact = Toad::Vec2f{0, 0};
            for (const Toad::Vec2f& pt : contacts)
                contact += pt;

            contact /= (float)contacts.size();
        }
        else
        {
             // falback
            contact = (a.rb.center + b.rb.center) * 0.5f;
        }
    
        return true;
    }

    void Sim::Update(float dt )
    {
        for (Spring& spr : springs)
        {
            spr.Update(dt);
        }
    
        for (Polygon& p : polygons)
        {
            if (p.rb.is_static)
            {
                p.rb.velocity = Toad::Vec2f{0, 0};
                p.rb.angular_velocity = 0.f;
                continue;
            }

            p.rb.velocity += gravity * dt;
            
            Toad::Vec2f center_prev = p.rb.center; 
            p.rb.Update(dt);
            Toad::Vec2f movement = p.rb.center - center_prev;
            p.UpdateCentroid();
            p.Translate(movement);
            p.Rotate(p.rb.angular_velocity * dt);
        }

        for (int i = 0; i < polygons.size(); i++)
        {
            for (int j = i + 1; j < polygons.size(); j++)
            {
                Toad::Vec2f contact, normal;
                float penetration;
                
                bool collide = SAT(polygons[i], polygons[j], normal, penetration, contact);
                
                DrawText("Collision: {}", collide);
                DrawText("Penetration: {}", penetration);

                Toad::DrawingCanvas::DrawArrow(contact, normal * 10.f, 0.5f);

                if (collide) 
                {
                    // for (int k = 0; k < 10; k++)
                        Resolve(polygons[i].rb, polygons[j].rb, contact, normal, penetration); 
                }
            }
        }
    }

    void Sim::Resolve(Rigidbody& a, Rigidbody& b, const Toad::Vec2f& contact, const Toad::Vec2f& normal, float penetration)
    {
        float mass_sum = a.mass + b.mass;

        if (abs(mass_sum) <= FLT_EPSILON)
            return;

        float move_a = (b.mass / mass_sum) * penetration;
        float move_b = (a.mass / mass_sum) * penetration;

        if (!a.is_static)
            a.center -= normal * move_a;
        if (!b.is_static)
            b.center += normal * move_b;

        Toad::Vec2f diff_a = contact - a.center;
        Toad::Vec2f diff_b = contact - b.center;

        Toad::Vec2f rel_vel = b.velocity - a.velocity;
        float vel_along_normal = dot(rel_vel, normal);

        if (vel_along_normal > 0) 
            return; 

        float e = (a.restitution + b.restitution) / 2.f;
        float j = -(1.f + e) * vel_along_normal / (1.f / a.mass + 1.f / b.mass);

        Toad::Vec2f impulse = normal * j;

        if (!a.is_static)
            a.velocity -= impulse / a.mass;
        if (!b.is_static)
            b.velocity += impulse / b.mass;

        // detect sliding and use friction 
        Toad::Vec2f perp = {-normal.y, normal.x};
        
        // Toad::DrawingCanvas::DrawArrow(contact, perp * 10, 1.f);
        // a.slide = dot(perp, normalize(a.velocity));
        // b.slide = dot(perp, normalize(b.velocity));
        // // LOGDEBUGF("PERP: {},{} {},{} A {} B {}",perp.x, perp.y, a.velocity.x, a.velocity.y, vel_perp_a, vel_perp_b);
        // a.velocity += a.slide * a.friction;
        // b.velocity -= b.slide * b.friction;
        // // DrawText("{} {}", a.slide, a.friction);

        // a.velocity *= a.friction;
        // b.velocity *= b.friction;

        float torque_a = cross(diff_a, impulse); 
        float torque_b = cross(diff_b, impulse);

        if (!a.is_static)
        {
            a.angular_velocity -= torque_a / a.moment_of_inertia;
            
            const float angular_velocity_factor = 10.f;
            Toad::Vec2f vel_rot_diff = a.velocity - (perp * (a.angular_velocity * -angular_velocity_factor));
            float grip = std::max(penetration, 1.1f) * ((a.friction + b.friction) / 2.f);

            a.velocity -= vel_rot_diff * grip;
        }
        if (!b.is_static)
            b.angular_velocity += torque_b / b.moment_of_inertia;

        if (penetration > 0.01f)
        {
            Toad::Vec2f correction = normal * (penetration * 0.5f);
            DrawText("CORRECTING: {} {}", correction.x, correction.y);
            
            // apply corection and also check for resting
            // #todo change resting check 
            if (!a.is_static || !a.resting) 
            {
                a.center_correction = -correction;
                if (a.velocity_average.Length() < 0.3f && a.angular_velocity_average < 0.1f)
                {
                    a.resting = true;
                }
            }
            if (!b.is_static || !b.resting)
            {
                b.center_correction = correction;
                if (b.velocity_average.Length() < 0.3f && b.angular_velocity_average < 0.1f)
                    b.resting = true;
            }
        }
    }

    fz::Polygon& Sim::AddPolygon(fz::Polygon &polygon)
    {
        polygon.sim = this;
        polygons.emplace_back(polygon);
        return polygons.back();
    }

    fz::Spring& Sim::AddSpring(Polygon &start, Polygon &end, const Toad::Vec2f &rel_start, const Toad::Vec2f rel_end)
    {
        fz::Spring spring;         
        spring.start_rb = &start.rb;
        spring.start_rel = rel_start;
        spring.end_rb = &end.rb;
        spring.end_rel = rel_end;
        spring.target_len = fz::dist(end.rb.center + rel_end, start.rb.center + spring.start_rel);
        spring.min_len = spring.target_len / 3.f;
        LOGDEBUGF("{} {} | {} {}", rel_start.x, rel_start.y, rel_end.x, rel_end.y);
        springs.push_back(spring);

        // fz::Spring& res = springs.back();
        // use index this will eventually crash 
        // start.extra_points.emplace_back(&res.start_rel);
        // end.extra_points.emplace_back(&res.end_rel);

        start.attached_spring_points.emplace_back(springs.size() - 1, false);
        end.attached_spring_points.emplace_back(springs.size() - 1, true);
        return springs.back();
    }
}