#include "pch.h"
#include "DrawingCanvas.h"

#include "engine/default_objects/UI/Text.h" // GetDefaultFont

#include "engine/Logger.h"

namespace Toad
{

DrawingCanvas::DrawingCanvas()
{
    canvases.insert(this);
}

DrawingCanvas::~DrawingCanvas()
{
    canvases.erase(this);
}

void DrawingCanvas::DrawText(const Vec2f& position, std::string_view text, uint32_t char_size)
{
    sf::Text t(GetDefaultFontResource(), text.data());
    t.setPosition(position);
    t.setCharacterSize(char_size);
    
    text_to_draw.emplace_back(t);
}

void DrawingCanvas::DrawArrow(const Vec2f& position, const Vec2f& direction, float width, Color color)
{
    std::array<sf::Vertex, 3> hat;
    std::array<sf::Vertex, 4> rect;
    
    Vec2f perp = direction.Normalize();
    perp = Vec2f{-perp.y, perp.x};
    
    for (sf::Vertex& v : rect)
        v.color = color;
    for (sf::Vertex& v : hat)
        v.color = color;
    
    rect[0].position = position - (perp * width) / 2.f;
    rect[1].position = position + (perp * width) / 2.f;
    rect[2].position = rect[0].position + direction;
    rect[3].position = rect[1].position + direction;
    
    hat[0].position = rect[2].position - perp;
    hat[1].position = position + direction; // top
    hat[2].position = rect[3].position + perp;
    
    arrows_to_draw.emplace_back(position, direction, rect, hat);
}

void DrawingCanvas::DrawRect(const Vec2f& min, const Vec2f& max)
{
    std::array<sf::Vertex, 5> rect;

    rect[0].position = min;
    rect[1].position = {max.x, min.y};
    rect[2].position = max;
    rect[3].position = {min.x, max.y};
    rect[4].position = min;

    rects_to_draw.emplace_back(rect);
}

void DrawingCanvas::ClearDrawBuffers()
{
    text_to_draw.clear();
    arrows_to_draw.clear();
    rects_to_draw.clear();
}

// void DrawingCanvas::DrawVerticesAllCanvases(sf::RenderTarget& target, sf::PrimitiveType type)
// {
//     for (DrawingCanvas* dc : canvases)
//         dc->DrawVertices(target, type);
// }

// void DrawingCanvas::DrawBuffersAllCanvases(sf::RenderTarget &target)
// {
//     for (DrawingCanvas* dc : canvases)
//         dc->DrawBuffers(target);
// }

void DrawingCanvas::ModifyVertex(uint32_t i, uint32_t j, const sf::Vertex &v)
{
    vertices_to_draw[i][j] = v;
}

size_t DrawingCanvas::AddVertexArray(size_t i)
{
    vertices_to_draw.emplace_back();
    vertices_to_draw.back().resize(i, sf::Vertex{ {0, 0}, sf::Color::White, {0, 0} });
    return vertices_to_draw.size() - 1;
}

void DrawingCanvas::ClearVertices()
{
    vertices_to_draw.clear();
}

void DrawingCanvas::DrawVertices(sf::RenderTarget& target, sf::PrimitiveType type)
{
    for (const auto& v : vertices_to_draw)
        target.draw(v.data(), v.size(), type);
}

void DrawingCanvas::DrawBuffers(sf::RenderTarget& target)
{
    for (const DrawInfoText& txt : text_to_draw)
    {
        target.draw(txt.text);
    }
    
    for (const DrawInfoArrow& arrow : arrows_to_draw)
    {
        target.draw(arrow.rect.data(), arrow.rect.size(), sf::PrimitiveType::TriangleStrip);
        target.draw(arrow.hat.data(), arrow.hat.size(), sf::PrimitiveType::Triangles);
    }
    
    for (const DrawInfoRect& rect : rects_to_draw)
    {
        target.draw(rect.rect.data(), rect.rect.size(), sf::PrimitiveType::LineStrip);
    }
}

std::vector<sf::Vertex>& DrawingCanvas::CreateVA(size_t size)
{
    created_vertexarrays.emplace_back();
    created_vertexarrays.back().resize(size);
    return created_vertexarrays.back();
}
    
} // namespace Toad
