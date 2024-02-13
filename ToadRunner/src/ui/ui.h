#pragma once

namespace ui
{
	void engine_ui(ImGuiContext* ctx);

	void event_callback(const sf::Event& e);

	void HelpMarker(const char* desc);
}

namespace ImGui {
	bool SliderVec2(std::string_view label, float* x, float* y, float min = 0, float max = 0);
	bool SliderVec2(std::string_view label, Vec2f* v, float min = 0, float max = 0);	
	bool SliderVec2i(std::string_view label, int* x, int* y, int min = 0, int max = 0);
	bool SliderVec2i(std::string_view label, Vec2i* v, int min = 0, int max = 0);
}
