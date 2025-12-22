#include "framework/Framework.h"

#include "FZWorld.h"

#include "FZMath.h"
#include "FZCollission.h"

#include "scripts/Sim.h"

#include <future> 
#include <thread>
#include <array>

namespace fz
{
    using namespace Toad; 

    static void Resolve(Rigidbody& a, Rigidbody& b, const Vec2f& contact, const Vec2f& normal, float overlap)
    {
        Vec2f diff_a = contact - a.center;
        Vec2f diff_b = contact - b.center;

        Vec2f rel_vel = b.velocity - a.velocity;
        float vel_along_normal = dot(rel_vel, normal);

        if (vel_along_normal > 0) 
        {
            // skip impulse only apply correction
            Vec2f correction = normal * (overlap * 0.5f);
            a.center_correction = -correction;
            b.center_correction = correction;
            return;
        }

        float e = (a.restitution + b.restitution) / 2.f;
        float j = -(1.f + e) * vel_along_normal / (a.inv_mass + b.inv_mass);

        Vec2f impulse = normal * j;

        DrawingCanvas::DrawArrow(contact, normal);
        DrawingCanvas::DrawArrow(contact, impulse);

        a.velocity -= impulse * a.inv_mass;
        b.velocity += impulse * b.inv_mass;

        float torque_a = cross(diff_a, impulse); 
        float torque_b = cross(diff_b, impulse);

        // Vec2f perp = {-normal.y, normal.x};
        a.angular_velocity -= torque_a / a.moment_of_inertia;
        b.angular_velocity += torque_b / b.moment_of_inertia;
        // float grip = (a.friction + b.friction) / 2.f;

        Vec2f perp = {-normal.y, normal.x};
        a.angular_velocity -= torque_a / a.moment_of_inertia;
        const float angular_velocity_factor = 10.f;
        Vec2f vel_rot_diff = a.velocity - (perp * (a.angular_velocity * -angular_velocity_factor));
        float grip = std::max(overlap, 1.f) * ((a.friction + b.friction) / 2.f);
        a.velocity -= vel_rot_diff * grip;


        // Vec2f vel_rot_diff = a.velocity - diff_a.perpendicular() * (perp * (a.angular_velocity));
        // a.velocity += vel_rot_diff * grip;
        // a.angular_velocity *= 0.985f;

        // vel_rot_diff = b.velocity + diff_b.perpendicular() * (perp * (b.angular_velocity));
        // b.velocity -= vel_rot_diff * grip;
        // b.angular_velocity *= 0.985f;

        Vec2f correction = normal * (overlap * 0.5f);
        // DrawText("CORRECTING: {} {}", correction.x, correction.y);
        
        // apply corection and also check for resting

        // wake up sleepers
        if (a.is_sleeping)
        {
            a.sleeping_ticks = 0;
            a.is_sleeping = false;
        }
        if (b.is_sleeping)
        {
            b.sleeping_ticks = 0;
            b.is_sleeping = false;       
        }

        a.center_correction = -correction;
        b.center_correction = correction;
    }

    static void BruteForce(std::vector<Polygon>& polygons)
    {
        Vec2f normal;
        float overlap;
        size_t polygons_count = polygons.size(); 

        for (size_t i = 0; i < polygons_count; i++)
        {
            Polygon& a = polygons[i];

            for (size_t j = i + 1; j < polygons_count; j++)
            {
                Polygon& b = polygons[j];

                if (a.rb.is_static && b.rb.is_static)
                    continue;
                if (a.rb.is_sleeping && b.rb.is_sleeping)
                    continue;
                if (a.rb.is_sleeping && b.rb.is_static)
                    continue;
                
                size_t contact_count = 0;
                Vec2f contact;
                bool collide = SAT(a, b, normal, overlap, contact, contact_count);
                
                // DrawText("Collision: {}", collide);
                // DrawText("Overlap: {}", overlap);

                if (collide) 
                    Resolve(a.rb, b.rb, contact, normal, overlap); 
            }
        }
    }

    //
    // SWEEP AND PRUNE USING X AXIS 
    // 
    // This will also immediately solve 
    // #TODO: Springs won't work, they store index 
    static bool SortByLeftAxis(const Polygon& a, const Polygon& b)
    {
        return a.aabb.min.x < b.aabb.min.x;
    }

    static void SweepAndPrune(std::vector<Polygon>& polygons)
    {
        // sort by left axis 
        std::ranges::sort(polygons, SortByLeftAxis);

        Vec2f normal; 
        float overlap; 
        size_t polygons_count = polygons.size();

        for (size_t i = 0; i < polygons_count; i++)
        {
            Polygon& a = polygons[i];

            for (size_t j = i + 1; j < polygons_count; j++)
            {
                Polygon& b = polygons[j];

                if (a.rb.is_static && b.rb.is_static)
                    continue;
                if (a.rb.is_sleeping && b.rb.is_sleeping)
                    continue;
                if (a.rb.is_sleeping && b.rb.is_static)
                    continue;
                
                if (b.aabb.min.x > a.aabb.max.x)
                    break;

                // perform sat 
                size_t contact_count = 0;
                Vec2f contact;
                bool collide = SAT(a, b, normal, overlap, contact, contact_count);
                
                if (collide) 
                    Resolve(a.rb, b.rb, contact, normal, overlap); 
            }
        }
    }

    //
    // SWEEP AND PRUNE MULTITHREADED 
    //

    struct CollissionPair
    {
        Polygon* a;
        Polygon* b;

        // SAT results 
        Vec2f normal; 
        Vec2f contact;
        float overlap;
        
        char pad[24];
    };

    static std::vector<CollissionPair> collission_pairs;
    static std::vector<std::future<void>> workers;
    static std::mutex m;

    static void SweepAndPrunePart(std::vector<Polygon>& polygons, size_t start, size_t end)
    {           
        CollissionPair p;

        for (size_t i = start; i < end; i++)
        {
            Polygon& a = polygons[i];

            for (size_t j = i + 1; j < polygons.size(); j++)
            {
                Polygon& b = polygons[j];

                if (a.rb.is_static && b.rb.is_static)
                    continue;
                if (a.rb.is_sleeping && b.rb.is_sleeping)
                    continue;
                if (a.rb.is_sleeping && b.rb.is_static)
                    continue;
                
                if (b.aabb.min.x > a.aabb.max.x)
                    break;
                
                size_t contact_count = 0;
                bool collide = SAT(a, b, p.normal, p.overlap, p.contact, contact_count);
                if (collide)
                {
                    p.a = &a;
                    p.b = &b;
                    std::unique_lock lock(m);
                    collission_pairs.emplace_back(p);
                }
            }
        }
    }

    static void SweepAndPruneMT(std::vector<Polygon>& polygons, unsigned worker_count = 0)
    {
        std::ranges::sort(polygons, SortByLeftAxis);

        if (!worker_count)
            worker_count = std::thread::hardware_concurrency() / 2;

        size_t base = polygons.size() / worker_count;
        size_t remainder = polygons.size() % worker_count;
        // LOGDEBUGF("poly:{} base:{} rema:{} work:{}", polygons.size(), base, remainder, worker_count);

        size_t start = 0;
        size_t end = start + base;

        workers.clear();
        collission_pairs.clear();

        for (unsigned i = 0; i < worker_count; i++)
        {
            size_t end = start + base + (i < remainder ? 1 : 0);
            if (start >= end) // in case where worker_count > polygons.size()
                break;

            workers.emplace_back(std::async(SweepAndPrunePart, std::ref(polygons), start, end));
            start = end;
        }

        for (auto& worker : workers)
            worker.get();

        for (auto& pair : collission_pairs)
        {
            Resolve(pair.a->rb, pair.b->rb, pair.contact, pair.normal, pair.overlap); 
        }
    }

    World::World()
    {
        ResetRenderingState();
    }

    void World::Update(float dt)
    {
        for (Spring& spr : springs)
        {
            spr.Update(dt);
        }
        for (Thruster& thr : thrusters)
        {
            thr.Update(dt);
        }

        BruteForce(polygons);
        // SweepAndPrune(polygons);
        // SweepAndPruneMT(polygons, 2);

        for (int i = 0; i < polygons.size(); i++)
        {   
            Polygon& p = polygons[i];

            if (p.rb.is_static || p.rb.is_sleeping)
            {
                p.rb.velocity = Vec2f{0, 0};
                p.rb.angular_velocity = 0.f;
                UpdatePolygonVertices(i);
                continue;
            }

            p.rb.velocity += gravity * dt;
            
            Vec2f center_prev = p.rb.center; 
            p.rb.Update(dt);
            Vec2f movement = p.rb.center - center_prev;
            p.UpdateCentroid();
            p.Translate(movement);
            p.Rotate(p.rb.angular_velocity * dt);

            UpdatePolygonVertices(i);
        }
    }

    void World::UpdatePolygonVertices(uint32_t polygon_index)
    {
        const Polygon& p = polygons[polygon_index];
        float mass_col = (uint8_t)p.rb.inv_mass * 100;
        Color color(255, mass_col, mass_col, 255);

        for (int j = 0; j < p.vertices.size(); j++)
        {
            sf::Vertex v;
            v.position = p.vertices[j];
            v.color = color;
            dc.ModifyVertex(polygon_index, j, v);
        }
    }


    size_t World::AddPolygon(fz::Polygon& polygon)
    {
        dc.AddVertexArray(polygon.vertices.size());
        polygon.world = this;
        polygon.id = polygons.size();
        polygons.emplace_back(polygon);
        return polygon.id;
    }

    void World::RemovePolygon(fz::Polygon &polygon)
    {
        polygons.erase(polygons.begin() + polygon.id);
        for (size_t i = polygon.id; i < polygons.size(); i++)
            polygons[i].id--;
    }

    fz::Spring& World::AddSpring(Polygon& start, Polygon& end, const Vec2f& rel_start, const Vec2f& rel_end)
    {
        fz::Spring spring;         
        spring.start_rb = &start.rb;
        spring.start_rel = rel_start;
        spring.end_rb = &end.rb;
        spring.end_rel = rel_end;
        spring.target_len = fz::dist(end.rb.center + rel_end, start.rb.center + spring.start_rel);
        spring.min_len = spring.target_len / 3.f;
        // LOGDEBUGF("{} {} | {} {}", rel_start.x, rel_start.y, rel_end.x, rel_end.y);
        springs.push_back(spring);

        // fz::Spring& res = springs.back();
        // use index this will eventually crash 
        // start.extra_points.emplace_back(&res.start_rel);
        // end.extra_points.emplace_back(&res.end_rel);

        start.attached_spring_points.emplace_back(springs.size() - 1, false);
        end.attached_spring_points.emplace_back(springs.size() - 1, true);
        return springs.back();
    }

    size_t World::AddThruster(Polygon& p, const Vec2f& rel)
    {
        return AddThruster(p.id, rel);
    }

    size_t World::AddThruster(size_t p_id, const Vec2f& rel)
    {
        fz::Thruster t;
        t.world = this;
        t.id = thrusters.size();
        t.attached_polygon = p_id;
        t.attached_rel_pos = rel;
        t.SetDirection(180);
        thrusters.push_back(t);

        return t.id;
    }

    void World::ResetRenderingState()
    {
	    dc.ClearVertices();
    }
}


