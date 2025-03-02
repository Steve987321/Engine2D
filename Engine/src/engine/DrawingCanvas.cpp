#include "pch.h"
#include "DrawingCanvas.h"

namespace Toad
{
	static std::vector<std::vector<sf::Vertex>> vertices_to_draw;
	static std::vector<std::vector<sf::Vertex>> created_vertexarrays;

	static std::vector<sf::Vertex>& CreateVA(size_t size)
	{
		created_vertexarrays.emplace_back();
		created_vertexarrays.back().resize(size);
		return created_vertexarrays.back();
	}

	void DrawingCanvas::ModifyVertex(uint32_t i, uint32_t j, const sf::Vertex& v)
	{
		vertices_to_draw[i][j] = v;
	}

	void DrawingCanvas::AddVertexArray(size_t i)
	{
		vertices_to_draw.emplace_back();
		vertices_to_draw.back().resize(i, sf::Vertex{ {0, 0}, sf::Color::White, {0, 0} });
	}

	void DrawingCanvas::Clear()
	{
		vertices_to_draw.clear();
	}

	void DrawingCanvas::DrawVertices(sf::RenderTarget& target)
	{
		for (const auto& v : vertices_to_draw)
		{
			target.draw(v.data(), v.size(), sf::Points);
		}
	}

}
