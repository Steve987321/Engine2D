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
	ENGINE_API void Clear();
	ENGINE_API void DrawVertices(sf::RenderTarget& target);
};

}
