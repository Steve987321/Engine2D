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

ENGINE_API void DefaultViewportUI();

struct Viewport
{
    std::function<void()> ui;
    std::unique_ptr<sf::RenderWindow> window;
};

class AppWindow : public sf::RenderWindow
{
public:
	ENGINE_API bool Create(uint32_t window_width, uint32_t window_height, uint32_t frame_limit = 60, uint32_t style = 0, std::string window_name = "");
	ENGINE_API ImGuiContext* GetImGuiContext();

	ENGINE_API void RecreateImGuiContext();
	ENGINE_API static void AddViewport(const sf::VideoMode& mode, std::string_view title, uint32_t style, std::function<void()> ui = DefaultViewportUI);

	ENGINE_API static void UpdateViewports(const FEVENT_CALLBACK& callback);

	ENGINE_API static std::queue<std::filesystem::path>& GetDroppedFilesQueue();

	ENGINE_API static void CleanUpViewports();

#ifdef _WIN32
	static inline LONG_PTR orginal_wnd_proc = NULL;
	static LRESULT CALLBACK WndProc(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);
#endif

private:
	using sf::RenderWindow::create;

	static inline std::queue<std::filesystem::path> dropped_files_queue {};

	static inline std::vector<Viewport> viewports {};
	ImGuiIO* io = nullptr;
};

}