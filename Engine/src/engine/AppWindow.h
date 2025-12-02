#pragma once

#include "imgui/imgui_internal.h"
#include "EngineCore.h"
#include "engine/Types.h"

namespace sf
{
	class Event;
	class RenderWindow;
}

namespace Toad
{
	struct AppSettings;
	class Camera;

class ENGINE_API AppWindow : public sf::RenderWindow
{
public:
	bool Create(uint32_t window_width, uint32_t window_height, uint32_t frame_limit = 60, uint32_t style = 0, std::string window_name = "");
	ImGuiContext* GetImGuiContext();

	void RecreateImGuiContext();
	static void AddViewport(const sf::VideoMode& mode, std::string_view title, uint32_t style);

	static void UpdateViewports(const FEVENT_CALLBACK& callback);

	static std::queue<std::filesystem::path>& GetDroppedFilesQueue();

    static void CleanUpViewports();

#ifdef _WIN32
	static inline LONG_PTR orginal_wnd_proc = NULL;
	static LRESULT CALLBACK WndProc(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);
#endif

private:
	using sf::RenderWindow::create;

	static inline std::queue<std::filesystem::path> dropped_files_queue {};

	static inline std::vector<std::unique_ptr<sf::RenderWindow>> viewports {};
	ImGuiIO* io = nullptr;
};

}