#include "framework/Framework.h"
#include "SandBoxEnvironment.h"

#include "scripts/Sim.h"
#include "Fizzix/FZWorld.h"
#include "Fizzix/FZMath.h"

namespace SimEnvironments
{
    void SandBoxEnvironmetLoad()
    {
        fz::World& world = Sim::GetWorld();
        world.gravity.y = 0;
        auto square_vertices = fz::CreateSquare(50, 50);

        world.polygons.clear();
        world.springs.clear();
        
        fz::Polygon p1({square_vertices.begin(), square_vertices.end()});
        fz::Polygon p2({square_vertices.begin(), square_vertices.end()});
        
        p1.Rotate(20.f);
        p2.Rotate(10.f);

        p1.rb.velocity.x = 10.f;
        p2.rb.velocity.x = -10.f;
        p1.Translate({-50, 0});
        p2.Translate({50, 0});

        world.AddPolygon(p1);
        world.AddPolygon(p2);
    }
}

