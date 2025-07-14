#include "framework/Framework.h"
#include "CarEnvironment.h"

#include "scripts/Sim.h"
#include "Fizzix/FZSim.h"
#include "Fizzix/FZMath.h"

#include "Toot/TVM/TVM.h"

static size_t index_terrain = 0;
static size_t index_car_body = 0;
static size_t index_car_wheel1 = 0;
static size_t index_car_wheel2 = 0;

void CarEnvironmentLoad()
{
    // #TODO: use sim.AddSpring
    
    fz::Sim& sim = Sim::GetSim();

    sim.polygons.clear();
    sim.springs.clear();
    sim.springs.reserve(10);

    // terrain (flat)
    std::array<Toad::Vec2f, 6> terrain_vertices = fz::CreateSquare(1000, 100);
    
    // car body
    std::array<Toad::Vec2f, 6> body_vertices = fz::CreateSquare(50, 20);
    
    // cars square wheels 
    std::array<Toad::Vec2f, 6> wheel_vertices = fz::CreateSquare(10, 10);

    // obstacle 
    std::array<Toad::Vec2f, 6> obstacle_vertices = fz::CreateSquare(5, 10);

    { 
        fz::Polygon terrain({terrain_vertices.begin(), terrain_vertices.end()});
        terrain.rb.is_static = true;
        terrain.rb.friction = 1.f;
        terrain.rb.restitution = 0.f;
        terrain.rb.mass = 0.1f;

        fz::Polygon car_body({body_vertices.begin(), body_vertices.end()});
        car_body.Translate({10, -50});
        car_body.rb.mass = 10.f;

        fz::Polygon car_wheel1({wheel_vertices.begin(), wheel_vertices.end()});
        car_wheel1.Translate({15, -20});
        car_wheel1.rb.friction = 1.f;
        car_wheel1.rb.mass = 50.f;
        car_wheel1.rb.restitution = 0.f;
        car_wheel1.rb.moment_of_inertia = 10000.f;

        fz::Polygon car_wheel2({wheel_vertices.begin(), wheel_vertices.end()});
        car_wheel2.Translate({45, -20});
        car_wheel2.rb.friction = 1.f;
        car_wheel2.rb.mass = 50.f;
        car_wheel2.rb.restitution = 0.f;
        car_wheel2.rb.moment_of_inertia = 10000.f;

        // obstacles
        fz::Polygon obstacle({obstacle_vertices.begin(), obstacle_vertices.end()});
        for (int i = 15; i < 20; i++)
        {
            for (int j = 1; j < 3; j++)
            {
                obstacle.Translate({(float)i * 5.f, (float)j * -5.f});
                sim.AddPolygon(obstacle);
            }
        }

        index_car_body = sim.polygons.size();
        sim.AddPolygon(car_body);

        index_car_wheel1 = sim.polygons.size();
        sim.AddPolygon(car_wheel1);

        index_car_wheel2 = sim.polygons.size();
        sim.AddPolygon(car_wheel2);

        // static object pushed last because how collision checks are done 
        index_terrain = sim.polygons.size();
        sim.AddPolygon(terrain);
    }

    fz::Rigidbody& car_wheel1_rb = sim.polygons[index_car_wheel1].rb;
    fz::Rigidbody& car_wheel2_rb = sim.polygons[index_car_wheel2].rb;
    fz::Rigidbody& car_body_rb = sim.polygons[index_car_body].rb;

    // attach springs 
    fz::Spring spr_wheels;
    spr_wheels.stiffness = 3.5f;
    spr_wheels.rotation_force_factor = 0.f;
    spr_wheels.start_rb = &car_wheel1_rb;
    spr_wheels.end_rb = &car_wheel2_rb;
    spr_wheels.target_len = fz::dist(car_wheel1_rb.center, car_wheel2_rb.center);
    sim.springs.emplace_back(spr_wheels);
    sim.polygons[index_car_wheel1].attached_spring_points.emplace_back(sim.springs.size() - 1, false);
    sim.polygons[index_car_wheel2].attached_spring_points.emplace_back(sim.springs.size() - 1, true);

    // wheels to body 
    fz::Spring spr_wheel_body;
    spr_wheel_body.end_rb = &car_body_rb;

    spr_wheel_body.stiffness = 1.f;
    spr_wheel_body.compression_damping = 0.5f;
    spr_wheel_body.rebound_damping = 0.9f;

    // wheel 1
    spr_wheel_body.start_rb = &car_wheel1_rb;
    spr_wheel_body.end_rel = Toad::Vec2f{-15, 5};
    spr_wheel_body.target_len = fz::dist(car_wheel1_rb.center, car_body_rb.center + spr_wheel_body.end_rel);
    sim.springs.emplace_back(spr_wheel_body);
    sim.polygons[index_car_wheel1].attached_spring_points.emplace_back(sim.springs.size() - 1, false);
    sim.polygons[index_car_body].attached_spring_points.emplace_back(sim.springs.size() - 1, true);

    // wheel 2
    spr_wheel_body.start_rb = &car_wheel2_rb;
    spr_wheel_body.end_rel = Toad::Vec2f{15, 5};
    sim.springs.emplace_back(spr_wheel_body);
    sim.polygons[index_car_wheel2].attached_spring_points.emplace_back(sim.springs.size() - 1, false);
    sim.polygons[index_car_body].attached_spring_points.emplace_back(sim.springs.size() - 1, true);

    // extra springs 
    // spr_wheel_body.stiffness = 1.f;
    // spr_wheel_body.compression_damping = 1.f;
    // spr_wheel_body.rebound_damping = 1.f;

    // wheel 1 extra 1
    spr_wheel_body.start_rb = &car_wheel1_rb;
    spr_wheel_body.end_rel = Toad::Vec2f{0,8};
    spr_wheel_body.target_len = fz::dist(car_wheel1_rb.center, car_body_rb.center + spr_wheel_body.end_rel);
    sim.springs.emplace_back(spr_wheel_body);
    sim.polygons[index_car_wheel1].attached_spring_points.emplace_back(sim.springs.size() - 1, false);
    sim.polygons[index_car_body].attached_spring_points.emplace_back(sim.springs.size() - 1, true);

    // wheel 2 extra 1
    spr_wheel_body.start_rb = &car_wheel2_rb;
    spr_wheel_body.end_rel = Toad::Vec2f{0,8};
    sim.springs.emplace_back(spr_wheel_body);
    sim.polygons[index_car_wheel2].attached_spring_points.emplace_back(sim.springs.size() - 1, false);
    sim.polygons[index_car_body].attached_spring_points.emplace_back(sim.springs.size() - 1, true);

    // wheel 1 extra 2
    spr_wheel_body.start_rb = &car_wheel1_rb;
    spr_wheel_body.end_rel = Toad::Vec2f{-25,5};
    spr_wheel_body.target_len = fz::dist(car_wheel1_rb.center, car_body_rb.center + spr_wheel_body.end_rel);
    sim.springs.emplace_back(spr_wheel_body);
    sim.polygons[index_car_wheel1].attached_spring_points.emplace_back(sim.springs.size() - 1, false);
    sim.polygons[index_car_body].attached_spring_points.emplace_back(sim.springs.size() - 1, true);

    // wheel 2 extra 2
    spr_wheel_body.start_rb = &car_wheel2_rb;
    spr_wheel_body.end_rel = Toad::Vec2f{25,5};
    sim.springs.emplace_back(spr_wheel_body);
    sim.polygons[index_car_wheel2].attached_spring_points.emplace_back(sim.springs.size() - 1, false);
    sim.polygons[index_car_body].attached_spring_points.emplace_back(sim.springs.size() - 1, true);
}

void CarEnvironmentUpdate(float gas)
{
    fz::Sim& sim = Sim::GetSim();

    Toad::Camera* cam = Toad::Camera::GetActiveCamera();
    cam->SetPosition(sim.polygons[index_car_body].rb.center);

    if (sim.polygons[index_car_wheel1].rb.angular_velocity > 10.f)
        return;

    sim.polygons[index_car_wheel1].rb.angular_velocity += gas;
    sim.polygons[index_car_wheel2].rb.angular_velocity += gas;
}
