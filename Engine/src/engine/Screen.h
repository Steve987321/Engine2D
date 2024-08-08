#include "engine/Types.h"

namespace Toad
{

class Camera;

namespace Screen
{
    Vec2f ScreenToWorld(const Vec2i& point);
    Vec2f ScreenToWorld(const Vec2i& point, const Camera& cam);
}

}
