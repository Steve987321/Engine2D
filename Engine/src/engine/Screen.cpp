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
        return tex.mapPixelToCoords(point);
    }
}
}
