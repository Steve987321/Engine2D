#include "pch.h"
#include "DrawingCanvas.h"

#include "engine/default_objects/UI/Text.h" // GetDefaultFont

#include "engine/Logger.h"

namespace Toad
{

namespace DrawingCanvas
{

static std::vector<std::vector<sf::Vertex>> vertices_to_draw;
static std::vector<std::vector<sf::Vertex>> created_vertexarrays;

struct DrawInfoText
{
    sf::Text text;
};

struct DrawInfoArrow
{
    Vec2f position;
    Vec2f direction;
    std::array<sf::Vertex, 4> rect;
    std::array<sf::Vertex, 3> hat;
};

static std::vector<DrawInfoText> text_to_draw;
static std::vector<DrawInfoArrow> arrows_to_draw;

static std::vector<sf::Vertex>& CreateVA(size_t size)
{
    created_vertexarrays.emplace_back();
    created_vertexarrays.back().resize(size);
    return created_vertexarrays.back();
}

// exposed for the engine
void DrawBuffers(sf::RenderTarget& target)
{
    for (const DrawInfoText& txt : text_to_draw)
    {
        target.draw(txt.text);
    }
    
    for (const DrawInfoArrow& arrow : arrows_to_draw)
    {
        target.draw(arrow.rect.data(), arrow.rect.size(), sf::TriangleStrip);
        target.draw(arrow.hat.data(), arrow.hat.size(), sf::Triangles);
    }
}

void ModifyVertex(uint32_t i, uint32_t j, const sf::Vertex& v)
{
    vertices_to_draw[i][j] = v;
}

void AddVertexArray(size_t i)
{
    vertices_to_draw.emplace_back();
    vertices_to_draw.back().resize(i, sf::Vertex{ {0, 0}, sf::Color::White, {0, 0} });
}

void ClearVertices()
{
    vertices_to_draw.clear();
}

void ClearDrawBuffers()
{
    text_to_draw.clear();
    arrows_to_draw.clear();
}


void DrawVertices(sf::RenderTarget& target, sf::PrimitiveType type)
{
    for (const auto& v : vertices_to_draw)
        target.draw(v.data(), v.size(), type);
}

void DrawText(Vec2f position, std::string_view text, uint32_t char_size)
{
    sf::Text t(text.data(), GetDefaultFontResource());
    t.setPosition(position.x, position.y);
    t.setCharacterSize(char_size);
    
    text_to_draw.emplace_back(t);
}

void DrawArrow(Vec2f position, Vec2f direction, float width, Color color)
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
    
} // namespace DrawingCanvas

} // namespace Toad
