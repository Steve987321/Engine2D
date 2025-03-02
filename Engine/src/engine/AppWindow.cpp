#include "pch.h"
#include "EngineCore.h"
#include "AppWindow.h"

#include "engine/Types.h"
#include "engine/Logger.h"
#include "engine/systems/Time.h"

#include "imgui-SFML.h"
#include "imgui.h"

namespace Toad
{

bool AppWindow::Create(uint32_t window_width, uint32_t window_height, uint32_t frame_limit, uint32_t style, std::string window_name)
{
#if defined(TOAD_EDITOR)
	LOGDEBUGF("[Engine] Loading editor window {}x{}", window_width, window_height);

	create(sf::VideoMode(window_width, window_height), "Engine 2D", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize, sf::ContextSettings());
	setFramerateLimit(30);

#ifdef _WIN32
	// drag drop 
	HWND window_handle = getSystemHandle();

	DragAcceptFiles(window_handle, TRUE);
	orginal_wnd_proc = SetWindowLongPtrA(window_handle, GWLP_WNDPROC, (LONG_PTR)AppWindow::WndProc);
	ShowWindow(window_handle, SW_NORMAL);
#else
	void* window_handle = window.getSystemHandle();
#endif

	bool res = ImGui::SFML::Init(*this, false);
	LOGDEBUGF("[Engine] ImGui SFML Init result: {}", res);
	io = &ImGui::GetIO();
	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io->IniFilename = NULL;

	// io->Fonts->Clear();
	io->Fonts->AddFontDefault();
	// io->Fonts->Build();
	if (!ImGui::SFML::UpdateFontTexture())
	{
		LOGERROR("[Engine] Failed to update font texture");
	}

	return res;
#else
	// TODO: CHENGE DEEZZ
	LOGDEBUG("[Engine] Creating window");
	create(sf::VideoMode(window_width, window_height), window_name, style, sf::ContextSettings{});
	setFramerateLimit(frame_limit);

#ifndef NDEBUG // imgui
	bool res = ImGui::SFML::Init(*this, false);
	LOGDEBUGF("[Engine] ImGui SFML Init result: {}", res);
	io = &ImGui::GetIO();
	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	// io->Fonts->Clear();
	io->Fonts->AddFontDefault();
	// io->Fonts->Build();
	ImGui::SFML::UpdateFontTexture();
#endif
	return true;
#endif
}

ImGuiContext* AppWindow::GetImGuiContext()
{
	return ImGui::GetCurrentContext();
}

void AppWindow::RecreateImGuiContext()
{
	ImGui::SFML::Shutdown();

	ImGui::CreateContext();

	bool res = ImGui::SFML::Init(*this, false);
	LOGDEBUGF("[Engine] ImGui SFML Init result: {}", res);
	io = &ImGui::GetIO();
	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io->IniFilename = NULL;

	//io->Fonts->Clear();
	io->Fonts->AddFontDefault();
	//io->Fonts->Build();
	if (!ImGui::SFML::UpdateFontTexture())
	{
		LOGERROR("[Engine] Failed to update font texture");
	}
}

void AppWindow::AddViewport(const sf::VideoMode& mode, std::string_view title, uint32_t style)
{
	auto window = std::make_shared<sf::RenderWindow>();
	window->create(mode, title.data(), style);
	window->setFramerateLimit(30);
	bool res = ImGui::SFML::Init(*window, true);
	LOGDEBUGF("[AddViewport] ImGui SFML Init result: {}", res);
	viewports.emplace_back(window);
}

void AppWindow::UpdateViewports(const FEVENT_CALLBACK& ecallback)
{
	auto it = viewports.begin();
	while (it != viewports.end())
	{
		bool erased = false;

		sf::Event e2;
		while (!erased && (*it)->pollEvent(e2))
		{
			ImGui::SFML::ProcessEvent(*(*it), e2);
			ecallback(e2);

			switch (e2.type)
			{
			case e2.Closed:
			{
				ImGui::SFML::Shutdown(*(*it));
				(*it)->close();
				it = viewports.erase(it);
				erased = true;
				break;
			}
			default:
				break;

			}
		}

		if (!erased)
		{
			ImGui::SFML::Update(*(*it), Time::GetDeltaTimeRaw());
			(*it)->clear(sf::Color::Black);
			ImGui::Begin("abc");
			ImGui::Text("ABC");

			ImGui::End();
			ImGui::SFML::Render(*(*it));
			(*it)->display();
			++it;
		}
	}
}

std::queue<std::filesystem::path>& AppWindow::GetDroppedFilesQueue()
{
	return dropped_files_queue;
}

#ifdef _WIN32
LRESULT CALLBACK AppWindow::WndProc(HWND handle, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (message == WM_DROPFILES)
	{
		HDROP hdrop = reinterpret_cast<HDROP>(wparam);
		POINT drag_point = { 0,0 };
		DragQueryPoint(hdrop, &drag_point);

		const UINT dropped_files_count = DragQueryFileA(hdrop, 0xFFFFFFFF, nullptr, 0);
		for (UINT i = 0; i < dropped_files_count; i++)
		{
			const UINT buf_size = DragQueryFileA(hdrop, i, nullptr, 0) + 1;
			char* buf = new char[buf_size];

			DragQueryFileA(hdrop, i, buf, buf_size);

			AppWindow::dropped_files_queue.emplace(buf);

			delete[] buf;
		}

		DragFinish(hdrop);
	}

	return CallWindowProcA(reinterpret_cast<WNDPROC>(orginal_wnd_proc), handle, message, wparam, lparam);
}
#endif

}
