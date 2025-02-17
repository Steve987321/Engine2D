#pragma once

#include "EngineCore.h"
#include "Types.h"

namespace Toad
{

	// draw things
class ENGINE_API DrawingCanvas
{
public:
	DrawingCanvas() = default;

	void ModifyVertex(uint32_t i, uint32_t j, const sf::Vertex& v);
	void AddVertexArray(size_t i);
	void Clear();
	void DrawVertices(sf::RenderTarget& target);

private:
	std::vector<std::vector<sf::Vertex>> vertices_to_draw;
};

}
