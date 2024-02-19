#include <utility>

#pragma once

namespace ui
{
	struct TileSpritePlacer {
		std::filesystem::path path;
		Vec2i size;

		sf::Texture tile_map;
		std::vector<sf::Sprite> tiles;

        TileSpritePlacer(std::filesystem::path  sheet_path, const Vec2i& sheet_split_size, const sf::Texture& tile_sheet_texture, const std::vector<sf::Sprite>& tiles)
        : path(std::move(sheet_path)), size(sheet_split_size), tile_map(tile_sheet_texture), tiles(tiles)
        {
        }
	};

	struct SheetTileData
	{
		char* path;
		sf::IntRect tex_rect{};
		Vec2f tex_size{};
	};

	inline bool show_grid = true;
	inline bool drag_snap = false;
	inline Vec2i grid_size = {80, 80};

	void engine_ui(ImGuiContext* ctx);

	void event_callback(const sf::Event& e);
	void editor_texture_draw_callback(sf::RenderTexture& texture);

	void HelpMarker(const char* desc);
}

namespace ImGui {
	bool SliderVec2(std::string_view label, float* x, float* y, float min = 0, float max = 0);
	bool SliderVec2(std::string_view label, Vec2f* v, float min = 0, float max = 0);	
	bool SliderVec2i(std::string_view label, int* x, int* y, int min = 0, int max = 0);
	bool SliderVec2i(std::string_view label, Vec2i* v, int min = 0, int max = 0);
}
