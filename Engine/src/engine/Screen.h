#include "engine/Types.h"

namespace Toad
{

class Camera;

namespace Screen
{
	ENGINE_API Vec2f ScreenToWorld(const Vec2i& point);
	ENGINE_API Vec2f ScreenToWorld(const Vec2i& point, const Camera& cam);
}

}
