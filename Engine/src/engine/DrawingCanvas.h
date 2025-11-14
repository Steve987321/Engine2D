#pragma once

#include "EngineCore.h"
#include "Types.h"

namespace Toad
{
    // draw things
class DrawingCanvas
{
public:
    ENGINE_API DrawingCanvas();
    ENGINE_API ~DrawingCanvas();

    // Draw text on the current frame
    ENGINE_API static void DrawText(const Vec2f& position, std::string_view text, uint32_t char_size = 20);
    ENGINE_API static void DrawArrow(const Vec2f& position, const Vec2f& direction, float width = 5.f, Color color = Color(255, 255, 0, 255));
    ENGINE_API static void DrawRect(const Vec2f& min, const Vec2f& max);
    ENGINE_API static void ClearDrawBuffers();
    ENGINE_API static std::unordered_set<DrawingCanvas*>& GetCanvases();

    // static void DrawVerticesAllCanvases(sf::RenderTarget& target, sf::PrimitiveType type = sf::PrimitiveType::Points);
    // static void DrawBuffersAllCanvases(sf::RenderTarget& target);

	ENGINE_API void ModifyVertex(uint32_t i, uint32_t j, const sf::Vertex& v);
	ENGINE_API size_t AddVertexArray(size_t i);
	ENGINE_API void ClearVertices();

    ENGINE_API void DrawVertices(sf::RenderTarget& target, sf::PrimitiveType type = sf::PrimitiveType::Points);
    
    // @note Should only be used by the engine runtime
    ENGINE_API void DrawBuffers(sf::RenderTarget& target);

    private: 
    struct DrawInfoRect
    {
        std::array<sf::Vertex, 5> rect;
    };

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

    static inline std::vector<DrawInfoText> text_to_draw;
    static inline std::vector<DrawInfoArrow> arrows_to_draw;
    static inline std::vector<DrawInfoRect> rects_to_draw;

    static inline std::unordered_set<DrawingCanvas*> canvases; 

    std::vector<std::vector<sf::Vertex>> vertices_to_draw;
    std::vector<std::vector<sf::Vertex>> created_vertexarrays;

    std::vector<sf::Vertex>& CreateVA(size_t size);
};
	
}
