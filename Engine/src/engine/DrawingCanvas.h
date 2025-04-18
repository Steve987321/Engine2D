#pragma once

#include "EngineCore.h"
#include "Types.h"

namespace Toad
{
	// draw things
namespace DrawingCanvas
{
	ENGINE_API void ModifyVertex(uint32_t i, uint32_t j, const sf::Vertex& v);
	ENGINE_API void AddVertexArray(size_t i);
	ENGINE_API void ClearVertices();
    ENGINE_API void DrawVertices(sf::RenderTarget& target, sf::PrimitiveType type = sf::Points);

    ENGINE_API void ClearDrawBuffers();

    // Draw text on the current frame
    ENGINE_API void DrawText(Vec2f position, std::string_view text, uint32_t char_size = 20);
    ENGINE_API void DrawArrow(Vec2f position, Vec2f direction, float width = 5.f, Color color = Color(255, 255, 0, 255));
};

}
