#include "Fizzix/FZSim.h"

namespace Toad
{
    class Sprite;
    class Vec2f; 
}

class PhysObj
{
public:
    void AddSquare(fz::Sim& sim, const Toad::Sprite& sprite);
    void AddPolygon(fz::Sim& sim, const std::vector<Toad::Vec2f>& vertices);

    void SetPosition(const Toad::Vec2f& pos);

    fz::Polygon* pg = nullptr;
};