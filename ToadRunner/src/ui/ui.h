#pragma once

namespace ui
{
	void engine_ui(ImGuiContext* ctx);

	void event_callback(const sf::Event& e);

	void HelpMarker(const char* desc);
}

namespace ImGui {
	void SliderVec2(std::string_view label, float* x, float* y, float min, float max);
	void SliderVec2(std::string_view label, Vec2f* v, float min, float max);
}
