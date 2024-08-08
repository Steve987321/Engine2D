#include "pch.h"
#include "engine/Engine.h"
#include "Screen.h"

namespace Toad
{
namespace Screen
{
    Vec2f ScreenToWorld(const Vec2i& point)
    {
        sf::RenderTexture& tex = Engine::Get().GetActiveRenderTexture();

        Camera* current_cam = Engine::Get().interacting_camera;
        if (!current_cam) 
            return {-1, -1};

        return ScreenToWorld(point, *current_cam);
    }

    Vec2f ScreenToWorld(const Vec2i& point, const Camera& cam)
    {
        sf::RenderTexture& tex = Engine::Get().GetActiveRenderTexture();
        float fx = cam.GetOriginalSize().x / cam.GetSize().x;
        float fy = cam.GetOriginalSize().y / cam.GetSize().y;
        Vec2i fixed_point = {(int)((float)point.x * fx), (int)(float)(point.y * fy)};
        return tex.mapPixelToCoords(fixed_point);
    }
}
}
