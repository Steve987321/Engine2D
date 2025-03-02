#include "engine/Types.h"

namespace Toad
{

class Camera;

namespace Screen
{
	ENGINE_API Vec2f ScreenToWorld(const Vec2i& point);
	ENGINE_API Vec2f ScreenToWorld(const Vec2i& point, const Camera& cam);
	ENGINE_API Vec2f WorldToScreen(const Vec2f& point, const Camera& cam, const Vec2f& content_size, const Vec2f& content_pos);
}

}
