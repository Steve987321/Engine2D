#include "framework/Framework.h"
#include "CarEnvironment.h"

#include "scripts/Sim.h"
#include "Fizzix/FZWorld.h"
#include "Fizzix/FZMath.h"

#include "Toot/TVM/TVM.h"

static size_t index_terrain = 0;
static size_t index_car_body = 0;
static size_t index_car_wheel1 = 0;
static size_t index_car_wheel2 = 0;

namespace SimEnvironments
{

void CarEnvironmentLoad()
{
    // #TODO: use world.AddSpring
    
    fz::World& world = Sim::GetWorld();

    world = fz::World{};
    world.springs.reserve(10);

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
        terrain.rb.inv_mass = 1.f;

        fz::Polygon car_body({body_vertices.begin(), body_vertices.end()});
        car_body.Translate({10, -50});
        car_body.rb.inv_mass = 1.f / 10.f;

        fz::Polygon car_wheel1({wheel_vertices.begin(), wheel_vertices.end()});
        car_wheel1.Translate({15, -20});
        car_wheel1.rb.friction = 1.f;
        car_wheel1.rb.inv_mass = 1.f / 30.f;
        car_wheel1.rb.restitution = 0.f;
        car_wheel1.rb.moment_of_inertia = 10000.f;
        car_wheel1.rb.angular_damping = 0.95f;

        fz::Polygon car_wheel2({wheel_vertices.begin(), wheel_vertices.end()});
        car_wheel2.Translate({45, -20});
        car_wheel2.rb.friction = 1.f;
        car_wheel2.rb.inv_mass = 1.f / 30.f;
        car_wheel2.rb.restitution = 0.f;
        car_wheel2.rb.moment_of_inertia = 10000.f;
        car_wheel2.rb.angular_damping = 0.95f;

        // obstacles
        // fz::Polygon obstacle({obstacle_vertices.begin(), obstacle_vertices.end()});
        // for (int i = 15; i < 20; i++)
        // {
        //     for (int j = 1; j < 3; j++)
        //     {
        //         obstacle.Translate({(float)i * 5.f, (float)j * -5.f});
        //         world.AddPolygon(obstacle);
        //     }
        // }

        index_car_body = world.polygons.size();
        world.AddPolygon(car_body);

        index_car_wheel1 = world.polygons.size();
        world.AddPolygon(car_wheel1);

        index_car_wheel2 = world.polygons.size();
        world.AddPolygon(car_wheel2);

        // static object pushed last because how collision checks are done 
        index_terrain = world.polygons.size();
        world.AddPolygon(terrain);
    }

    fz::Rigidbody& car_wheel1_rb = world.polygons[index_car_wheel1].rb;
    fz::Rigidbody& car_wheel2_rb = world.polygons[index_car_wheel2].rb;
    fz::Rigidbody& car_body_rb = world.polygons[index_car_body].rb;

    // attach springs 
    fz::Spring spr_wheels;
    spr_wheels.stiffness = 3.5f;
    spr_wheels.rotation_force_factor = 0.f;
    spr_wheels.start_rb = &car_wheel1_rb;
    spr_wheels.end_rb = &car_wheel2_rb;
    spr_wheels.target_len = fz::dist(car_wheel1_rb.center, car_wheel2_rb.center);
    world.springs.emplace_back(spr_wheels);
    world.polygons[index_car_wheel1].attached_spring_points.emplace_back(world.springs.size() - 1, false);
    world.polygons[index_car_wheel2].attached_spring_points.emplace_back(world.springs.size() - 1, true);

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
    world.springs.emplace_back(spr_wheel_body);
    world.polygons[index_car_wheel1].attached_spring_points.emplace_back(world.springs.size() - 1, false);
    world.polygons[index_car_body].attached_spring_points.emplace_back(world.springs.size() - 1, true);

    // wheel 2
    spr_wheel_body.start_rb = &car_wheel2_rb;
    spr_wheel_body.end_rel = Toad::Vec2f{15, 5};
    world.springs.emplace_back(spr_wheel_body);
    world.polygons[index_car_wheel2].attached_spring_points.emplace_back(world.springs.size() - 1, false);
    world.polygons[index_car_body].attached_spring_points.emplace_back(world.springs.size() - 1, true);

    // extra springs 
    // spr_wheel_body.stiffness = 1.f;
    // spr_wheel_body.compression_damping = 1.f;
    // spr_wheel_body.rebound_damping = 1.f;

    // wheel 1 extra 1
    spr_wheel_body.start_rb = &car_wheel1_rb;
    spr_wheel_body.end_rel = Toad::Vec2f{0,8};
    spr_wheel_body.target_len = fz::dist(car_wheel1_rb.center, car_body_rb.center + spr_wheel_body.end_rel);
    world.springs.emplace_back(spr_wheel_body);
    world.polygons[index_car_wheel1].attached_spring_points.emplace_back(world.springs.size() - 1, false);
    world.polygons[index_car_body].attached_spring_points.emplace_back(world.springs.size() - 1, true);

    // wheel 2 extra 1
    spr_wheel_body.start_rb = &car_wheel2_rb;
    spr_wheel_body.end_rel = Toad::Vec2f{0,8};
    world.springs.emplace_back(spr_wheel_body);
    world.polygons[index_car_wheel2].attached_spring_points.emplace_back(world.springs.size() - 1, false);
    world.polygons[index_car_body].attached_spring_points.emplace_back(world.springs.size() - 1, true);

    // wheel 1 extra 2
    spr_wheel_body.start_rb = &car_wheel1_rb;
    spr_wheel_body.end_rel = Toad::Vec2f{-25,5};
    spr_wheel_body.target_len = fz::dist(car_wheel1_rb.center, car_body_rb.center + spr_wheel_body.end_rel);
    world.springs.emplace_back(spr_wheel_body);
    world.polygons[index_car_wheel1].attached_spring_points.emplace_back(world.springs.size() - 1, false);
    world.polygons[index_car_body].attached_spring_points.emplace_back(world.springs.size() - 1, true);

    // wheel 2 extra 2
    spr_wheel_body.start_rb = &car_wheel2_rb;
    spr_wheel_body.end_rel = Toad::Vec2f{25,5};
    world.springs.emplace_back(spr_wheel_body);
    world.polygons[index_car_wheel2].attached_spring_points.emplace_back(world.springs.size() - 1, false);
    world.polygons[index_car_body].attached_spring_points.emplace_back(world.springs.size() - 1, true);
}

void CarEnvironmentUpdate(float gas)
{
    fz::World& world = Sim::GetWorld();

    Toad::Camera* cam = Toad::Camera::GetActiveCamera();
    cam->SetPosition(world.polygons[index_car_body].rb.center);

    fz::Polygon& car_wheel1 = world.polygons[index_car_wheel1];
    fz::Polygon& car_wheel2 = world.polygons[index_car_wheel2];

    if (car_wheel1.rb.angular_velocity > 10.f)
    {
        car_wheel1.rb.angular_velocity = 10.f;
        car_wheel2.rb.angular_velocity = std::min(car_wheel2.rb.angular_velocity, 10.f);
        return;
    }

    car_wheel1.rb.angular_velocity += gas;
    car_wheel2.rb.angular_velocity += gas;
}

}