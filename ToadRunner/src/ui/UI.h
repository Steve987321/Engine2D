#pragma once

#include "engine/Types.h"
#include <utility>

namespace Toad
{
	class GameAssetsBrowser;
}

namespace ui
{
	constexpr int i8_min = std::numeric_limits<int8_t>::min();
	constexpr int i8_max = std::numeric_limits<int8_t>::max();
	constexpr int i16_min = std::numeric_limits<int16_t>::min();
	constexpr int i16_max = std::numeric_limits<int16_t>::max();
	constexpr int i32_max = std::numeric_limits<int32_t>::max();

	// ids for popups: this is a fix for popups not opening sometimes 
	const static ImGuiID project_creation_popup_id = ImHashStr("CreateProject");
	const static ImGuiID project_load_popup_id = ImHashStr("LoadProject");
	const static ImGuiID project_package_popup_id = ImHashStr("PackageProject");
	const static ImGuiID project_settings_popup_id = ImHashStr("ProjectSettings");
	const static ImGuiID save_scene_popup_id = ImHashStr("SaveScene");
	const static ImGuiID scene_modify_popup_id = ImHashStr("SceneModifyPopup");

	// editor viewport size
	inline Toad::Vec2i viewport_size = {};

	// imgui ini formatted main tabs status buffers
	inline std::string ini_main_buf;
	inline std::string ini_anim_buf;

	// when to show '*' for unsaved changes
	inline bool saved_scene = true;

	// reload scripts when tabbing back in
	inline bool auto_reload = true;

	inline Toad::Object* selected_obj = nullptr;
	inline std::set<std::string> selected_objects = {};

	struct TileSpritePlacer 
	{
		std::filesystem::path path;
		Toad::Vec2i size;

		Toad::Texture tile_map;
		std::vector<sf::Sprite> tiles;

        TileSpritePlacer(std::filesystem::path  sheet_path, const Toad::Vec2i& sheet_split_size, const sf::Texture& tile_sheet_texture, const std::vector<sf::Sprite>& tiles)
        : path(std::move(sheet_path)), size(sheet_split_size), tile_map(tile_sheet_texture), tiles(tiles)
        {
        }
	};

	struct SheetTileData
	{
		char* path;
		Toad::IntRect tex_rect{};
		Toad::Vec2f tex_size{};
	};

	inline bool show_grid = false;
	inline bool drag_snap = true;
	inline Toad::Vec2i grid_size = {10, 10};

	// reloads the scene when stopping 
	inline bool reload_scene_on_stop = true;

	// function that will be called in the inspector window 
	inline std::function<void()> inspector_ui;
	
	void engine_ui(ImGuiContext* ctx);

	void update_ini();
	void event_callback(const sf::Event& e);
	void editor_texture_draw_callback(sf::RenderTexture& texture);

	void save_ini_files(int exit_code);

	void HelpMarker(const char* desc);
}

namespace ImGui {
	bool SliderVec2(std::string_view label, float* x, float* y, float min = 0, float max = 0);
	bool SliderVec2(std::string_view label, Toad::Vec2f* v, float min = 0, float max = 0);
	bool SliderVec2i(std::string_view label, int* x, int* y, int min = 0, int max = 0);
	bool SliderVec2i(std::string_view label, Toad::Vec2i* v, int min = 0, int max = 0);	
	//bool ColorPicker4(std::string_view label, sf::Color* color)
}

