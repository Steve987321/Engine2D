#include "framework/Framework.h"
#include "RocketEnvironment.h"

#include "scripts/Sim.h"
#include "Fizzix/FZWorld.h"
#include "Fizzix/FZMath.h"

#include "scripts/Sim.h"

namespace SimEnvironments
{

void RocketEnvironmentLoad()
{
    fz::World& world = Sim::GetWorld();
    auto square_vertices = fz::CreateSquare(50, 50);
    world.polygons.clear();
    world.springs.clear();
    world.thrusters.clear();

    fz::Polygon p1({square_vertices.begin(), square_vertices.end()});
    fz::Polygon p2({square_vertices.begin(), square_vertices.end()});
    
    p1.Rotate(20.f);
    p2.Rotate(10.f);

    p1.rb.velocity.x = 10.f;
    p2.rb.velocity.x = -10.f;
    p1.Translate({-50, 0});
    p2.Translate({50, 0});

    size_t p1_id = world.AddPolygon(p1);
    size_t p2_id =  world.AddPolygon(p2);

    world.AddThruster(p1_id, {0, 10});
    
}

}