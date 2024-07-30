#include "pch.h"

#ifndef NDEBUG
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "Engine.h"
#include "Settings.h"

#include "game_core/ScriptRegister.h"
#include "game_core/Game.h"

#include "engine/systems/Time.h"
#include "engine/systems/Timer.h"
#include "utils/Wrappers.h"
#include <imgui/imgui.h>
#include <imgui-SFML.h>

namespace Toad
{

using json = nlohmann::json;

Engine::Engine()
{
	s_Instance = this;

	GetDesktopDimensions(m_width, m_height);

	m_width = (int)((float)m_width * 0.7f);
	m_height = (int)((float)m_height * 0.7f);

	m_editorCam.SetPosition({ 0, 0 });
	m_editorCam.SetSize({ (float)m_width, (float)m_height});
}

Engine::~Engine() = default;

bool Engine::Init()
{
	LOGDEBUG("[Engine] Initializing Engine");

	m_currentPath = get_exe_path().parent_path();

	for (const auto& e : std::filesystem::recursive_directory_iterator(m_currentPath))
	{
		if (e.path().filename().string().find("Game") != std::string::npos && e.path().extension() == LIB_FILE_EXT)
		{
			UpdateGameBinPaths(e.path().filename().string(), e.path().parent_path().string());
		}
	}

	LoadGameScripts();

#ifndef TOAD_EDITOR
	// load settings
	for (const auto& e : std::filesystem::directory_iterator(m_currentPath))
	{
		if (e.path().filename() == "settings" && e.path().extension() == ".json")
		{
			
		}
	}

	Scene* starting_scene = nullptr;

	// get start scene and get game dl
	for (const auto& e : std::filesystem::recursive_directory_iterator(m_currentPath))
	{
		if (e.path().filename().string().find("Game") != std::string::npos && e.path().extension() == LIB_FILE_EXT)
		{
			UpdateGameBinPaths(e.path().filename().string(), e.path().parent_path().string());
		}

		if (!e.path().has_extension())
			continue;

		if (e.path().extension() == FILE_EXT_TOADSCENE)
		{
			Scene& s = LoadScene(e.path());

			std::string lower;
			for (char c : s.name)
			{
				lower += std::tolower(c);
			}

			if (lower.find("start") != std::string::npos)
			{
				starting_scene = &s;
			}
		}
		if (e.path().extension() == FILE_EXT_FSM)
		{
			std::ifstream f(e.path());
			json fsm_data;
			try
			{
				fsm_data = json::parse(f);
			}
			catch (std::exception& e)
			{
				LOGDEBUGF("[Engine] Failed to parse {}", e.path().string());
				continue;
			}

			FSM fsm = FSM::Deserialize(fsm_data);
			fs::path relative = fs::relative(path, m_currentPath);
			ResourceManager::GetFSMs().Add(relative.string(), fsm);
		}
	}
#endif

	// #TODO: change to a .ini or .json
	AppSettings gsettings;
	if (m_currDLL != nullptr)
	{
        auto get_game_settings = reinterpret_cast<get_game_settings_t*>(DLibGetAddress(m_currDLL, "get_game_settings"));
		gsettings = get_game_settings();
	}

	if (!InitWindow(gsettings))
		return false;
	 
#ifdef TOAD_EDITOR
	LOGDEBUG("[Engine] Creating window texture for viewport & game view");
	m_windowTexture.create(m_window.getSize().x, m_window.getSize().y);
	m_windowEditorCamTexture.create(m_window.getSize().x, m_window.getSize().y);
#else
	m_beginPlay = true;
	if (starting_scene)
		Scene::SetScene(starting_scene);
	else
		Scene::SetScene(&m_emptyScene);
#endif

	m_isRunning = true;

	return true;
}

void Engine::Run()
{
	while (m_window.isOpen())
	{
#ifdef TOAD_EDITOR
		if (!m_window.hasFocus())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
#else 
		relative_mouse_pos = sf::Mouse::getPosition(m_window);
#endif
		Time::UpdateDeltaTime();

		// handle events 
		EventHandler();

#if defined(TOAD_EDITOR) || !defined(NDEBUG)
		// update imgui sfml
		ImGui::SFML::Update(m_window, Time::m_deltaTime);
#endif 
#ifdef TOAD_EDITOR
		// update objects 
		if (m_beginPlay)
		{
			Scene::current_scene.Update();
		}
#else
		Scene::current_scene.Update();
#endif

		// render the window and contents
		Render();
	}

#ifndef TOAD_EDITOR
	Scene::current_scene.End(nullptr);
#endif

	CleanUp();
}

bool Engine::InitWindow(const AppSettings& settings)
{
#if defined(TOAD_EDITOR)
	LOGDEBUG("[Engine] Loading editor window");

	m_window.create(sf::VideoMode(m_width, m_height), "Engine 2D", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize, sf::ContextSettings());
	m_window.setFramerateLimit(30);

#ifdef _WIN32
	// drag drop 
	HWND window_handle = m_window.getSystemHandle();
    DragAcceptFiles(window_handle, TRUE);
	s_originalWndProc = SetWindowLongPtrA(window_handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));

	// maximize 
	ShowWindow(window_handle, SW_MAXIMIZE);
#else
    void* window_handle = m_window.getSystemHandle();
#endif

	bool res = ImGui::SFML::Init(m_window, false);
	LOGDEBUGF("[Engine] ImGui SFML Init result: {}", res);
	m_io = &ImGui::GetIO();
	m_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	m_io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	// m_io->Fonts->Clear();
	m_io->Fonts->AddFontDefault();
	// m_io->Fonts->Build();
	ImGui::SFML::UpdateFontTexture();

	return res;
#else
	// TODO: CHENGE DEEZZ
	LOGDEBUG("[Engine] Creating window");
	m_window.create(sf::VideoMode(settings.window_width, settings.window_height), settings.window_name, settings.style, settings.ctx_settings);
	m_window.setFramerateLimit(settings.frame_limit);

#ifndef NDEBUG // imgui
	bool res = ImGui::SFML::Init(m_window, false);
	LOGDEBUGF("[Engine] ImGui SFML Init result: {}", res);
	m_io = &ImGui::GetIO();
	m_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	m_io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	// m_io->Fonts->Clear();
	m_io->Fonts->AddFontDefault();
	// m_io->Fonts->Build();
	ImGui::SFML::UpdateFontTexture();
#endif
	return true;
#endif
}

void Engine::EventHandler()
{
	sf::Event e;
	while (m_window.pollEvent(e))
	{
#if defined(TOAD_EDITOR) || !defined(NDEBUG)
		ImGui::SFML::ProcessEvent(m_window, e);
#endif
#if defined(TOAD_EDITOR)
		m_eventCallback(e);
#endif
		switch (e.type)
		{
		
		case sf::Event::Closed:
		{
			StopGameSession();
			m_window.close(); 
			break;
		}

		case sf::Event::Resized:
		{
#ifdef TOAD_EDITOR
			// update window texture size to new window size
			m_windowTexture.create(m_window.getSize().x, m_window.getSize().y);
			//auto view = m_window.getDefaultView();
			//view.setSize()
			//m_window.setView(view);
				// shape match pixel to coords
#endif
			break;
		}

		case sf::Event::KeyPressed:
		{
			if (m_beginPlay)
				Input::InvokeKeyPressCallbacks(e.key.code);
			break;
		}

		case sf::Event::KeyReleased:
		{
			if (m_beginPlay)
				Input::InvokeKeyReleaseCallbacks(e.key.code);
			break;
		}

		case sf::Event::MouseButtonPressed:
		{
			if (m_beginPlay)
				Input::InvokeMousePressCallbacks(e.mouseButton.button);
			break;
		}	
		case sf::Event::MouseButtonReleased:
		{
			if (m_beginPlay)
				Input::InvokeMouseReleaseCallbacks(e.mouseButton.button);
			break;
		}

		}
	}
}

void Engine::Render()
{
	m_window.clear(sf::Color::Black); // window bg

	//--------------------draw------------------------//

	Camera* cam = Camera::GetActiveCamera();

//#if !defined(NDEBUG) && !defined(TOAD_EDITOR)
//	ImDrawList* draw = ImGui::GetWindowDrawList();
//	const char* err_msg = "NO CAMERA'S IN SCENE ARE AVAILABLE FOR RENDERING";
//	ImVec2 size = ImGui::CalcTextSize(err_msg) / 2;
//	draw->AddText({ m_window.getSize().x / 2 - size.x, m_window.getSize().y / 2 - size.y }, IM_COL32(255, 0, 0, 255), err_msg);
//#endif

#if defined(TOAD_EDITOR)
	m_windowTexture.clear();
	m_windowEditorCamTexture.clear();

	// Update scene to the texture so it can display on the (game) viewport 
	Scene::current_scene.Render(m_windowTexture);
	if (cam != nullptr)
	{
		m_windowTexture.setView(cam->GetView());
	}
	m_windowTexture.display();

	Scene::current_scene.Render(m_windowEditorCamTexture);

	if (m_editorTextureDrawCallback)
	{
		m_editorTextureDrawCallback(m_windowEditorCamTexture);
	}

	m_windowEditorCamTexture.setView(m_editorCam.GetView());
	m_windowEditorCamTexture.display();

	// imgui
	m_renderUI(ImGui::GetCurrentContext());

	for (auto& obj : Scene::current_scene.objects_all)
		for (auto& [name, script] : obj->GetAttachedScripts())
			script->OnImGui(obj.get(), ImGui::GetCurrentContext());

	ImGui::SFML::Render(m_window);
	auto it = m_viewports.begin();
	while (it != m_viewports.end())
	{
		bool erased = false;

		sf::Event e2;
		while (!erased && (*it)->pollEvent(e2))
		{
			ImGui::SFML::ProcessEvent(*(*it), e2);
			m_eventCallback(e2);

			switch (e2.type)
			{
			case e2.Closed:
				ImGui::SFML::Shutdown(*(*it));
				(*it)->close();
				it = m_viewports.erase(it);
				erased = true;
				break;
			}
		}

		if (!erased)
		{
			ImGui::SFML::Update(*(*it), Time::m_deltaTime);
			(*it)->clear(sf::Color::Black);
			ImGui::Begin("abc");
			ImGui::Text("ABC");

			ImGui::End();
			ImGui::SFML::Render(*(*it));
			(*it)->display();
			++it;
		}
	}
#else
#ifndef NDEBUG
	for (auto& obj : Scene::current_scene.objects_all)
		for (auto& [name, script] : obj->GetAttachedScripts())
			script->OnImGui(obj.get(), ImGui::GetCurrentContext());

	ImGui::SFML::Render(m_window);
#endif 
	Scene::current_scene.Render(m_window);

	if (cam != nullptr)
	{
		m_window.setView(cam->GetView());
	}
#endif
	//--------------------draw------------------------//

	m_window.display();
}

sf::Vector2i Engine::GetWindowPos() const
{
	return { this->m_window.getPosition().x, this->m_window.getPosition().y };
}

Engine& Engine::Get()
{
	return *s_Instance;
}

sf::RenderWindow& Engine::GetWindow()
{
	return m_window;
}

sf::RenderTarget& Engine::GetActiveRenderTarget()
{
#ifdef TOAD_EDITOR
	return m_windowTexture;
#else
	return m_window; 
#endif 
}

sf::RenderTexture& Engine::GetWindowTexture()
{
	return m_windowTexture;
}

sf::RenderTexture& Engine::GetEditorCameraTexture()
{
	return m_windowEditorCamTexture;
}

Toad::Camera& Engine::GetEditorCamera()
{
	return m_editorCam;
}

ImGuiContext* Engine::GetImGuiContext()
{
	return ImGui::GetCurrentContext();
}

bool Engine::GameStateIsPlaying() const
{
	return m_beginPlay;
}

void Engine::StartGameSession()
{
	if (m_beginPlay)
		return;

	m_beginPlay = true;
	Scene::current_scene.Start();
}

void Engine::StopGameSession()
{
	if (m_beginPlay)
		Scene::current_scene.End(nullptr);

	m_beginPlay = false;
}

void Engine::AddViewport(const sf::VideoMode& mode, std::string_view title, uint32_t style)
{
	auto window = std::make_shared<sf::RenderWindow>();
	window->create(mode, title.data(), style);
	window->setFramerateLimit(30);
	bool res = ImGui::SFML::Init(*window, true);
	LOGDEBUGF("[Engine::AddViewport] ImGui SFML Init result: {}", res);
	m_viewports.emplace_back(window);
}

Vec2f Engine::ScreenToWorld(const Vec2i& screen_pos)
{
#ifdef TOAD_EDITOR
	
	if (!interacting_camera || !interacting_texture)
	{
		LOGERRORF("[Engine] No interacting texture or camera: texture {} camera: {}", (void*)interacting_camera, (void*)interacting_texture);
		return { -1, -1 };
	}

	return interacting_texture->mapPixelToCoords({ (int)(screen_pos.x), (int)(screen_pos.y) }, interacting_camera->GetView());

	return {-1 , -1};
#else
	Camera* cam = Camera::GetActiveCamera();

	if (cam)
	{
		float fx = cam->GetSize().x / m_window.getSize().x;
		float fy = cam->GetSize().y / m_window.getSize().y;

		return m_window.mapPixelToCoords({ (int)(screen_pos.x * fx), (int)(screen_pos.y * fy) }, cam->GetView());
	}

	return { -1, -1 };
#endif
}

void Engine::UpdateGameBinPaths(std::string_view game_bin_file_name, std::string_view bin_path)
{
	game_bin_directory = bin_path;
	game_bin_file = game_bin_file_name;

	if (!game_bin_directory.ends_with(PATH_SEPARATOR))
	{
		game_bin_directory += PATH_SEPARATOR;
	}
}

void Engine::LoadGameScripts()
{
	namespace fs = std::filesystem;
	using object_script = struct { std::string script_name; ReflectVarsCopy reflection; };
	std::unordered_map <std::string, std::vector<object_script>> objects_with_scripts{};

	for (auto& script : m_gameScripts | std::views::values)
	{
		// #TODO: save name and give warning about lost scripts 
		if (script)
			free(script);
		script = nullptr;
	}

	for (auto& obj : Scene::current_scene.objects_all)
	{
		auto& scripts = obj->GetAttachedScripts();
		if (scripts.empty())
		{
			continue;
		}

		for (auto i = scripts.begin(); i != scripts.end();)
		{
			ReflectVarsCopy vars;
			i->second->GetReflection().Get().copy(vars);
			objects_with_scripts[obj->name].emplace_back(object_script{ i->first, vars });

			obj->RemoveScript(i->first);
			i = scripts.begin();
		}
	}

	// clear Input callbacks 
	Input::Clear();

	if (m_currDLL)
	{
		DLibFree(m_currDLL);
	}

	fs::path game_dll_path = game_bin_directory + game_bin_file;
	fs::path current_game_dll = game_bin_directory + LIB_FILE_PREFIX + "GameCurrent" + LIB_FILE_EXT;
	// fs::path current_game_dll = game_bin_directory + game_bin_file;

#ifdef TOAD_EDITOR
	if (fs::exists(current_game_dll))
	{
		if (fs::exists(game_dll_path))
		{
			if (!fs::remove(current_game_dll.c_str()))
			{
				LOGERRORF("Failed to remove file {}", current_game_dll);
			}
		}
	}
#endif

	//#ifdef _WIN32
	//#ifdef __APPLE__
	//    if (!game_bin_directory.empty())
	//    {
	//#endif
	if (!fs::exists(game_dll_path)) {
		LOGWARNF("Couldn't find game dll file, {}", game_dll_path);
	}
	else {
		try {
			fs::rename(game_dll_path, current_game_dll);
		}
		catch (fs::filesystem_error& e) {
			LOGERRORF("{}", e.what());
			return;
		}
	}
	//#ifdef __APPLE__
	//    }
	//#endif
	//#endif

	auto dll = DLibOpen(current_game_dll.string());
	if (!dll)
	{
		LOGERRORF("Couldn't load game dll file, {} : {}", current_game_dll, DLGetError());
		return;
	}

	m_currDLL = dll;

	auto registerScripts = reinterpret_cast<register_scripts_t*>(DLibGetAddress(dll, "register_scripts"));
	auto getScripts = reinterpret_cast<get_registered_scripts_t*>(DLibGetAddress(dll, "get_registered_scripts"));

	registerScripts();

	for (const auto& [b, n] : getScripts())
	{
		Script* script = (Script*)(b);
		LOGDEBUGF("Load game script: {}", script->GetName().c_str());
		LOGDEBUGF("[Engine] Alloc for script {} with size {}. Base size {}: n: {}", script->GetName(), sizeof(*script), sizeof(Toad::Script), n);
		void* p = malloc(n);
		memcpy(p, b, n);

		// this can be deleted after 
		assert(p && "Allocation failed for script, please rebuild script to be updated");
		m_gameScripts[script->GetName()] = (Script*)p;
		m_gameScripts[script->GetName()]->ExposeVars();
	}
	for (TGAME_SCRIPTS::iterator it = m_gameScripts.begin(); it != m_gameScripts.end();)
	{
		if (!it->second)
		{
			LOGWARNF("Script {} is now null and is getting removed", it->first.c_str());
			it = m_gameScripts.erase(it);
		}
		else
		{
			++it;
		}
	}

	// update scripts on object if it has an old version

	for (auto& obj : Scene::current_scene.objects_all)
	{
		if (!objects_with_scripts.contains(obj->name))
		{
			continue;
		}

		const auto& prev_obj_state = objects_with_scripts[obj->name];

		for (auto& [attached_script_name, old_reflection_vars] : prev_obj_state)
		{
			auto it = m_gameScripts.find(attached_script_name);
			if (it != m_gameScripts.end())
			{
				// update exposed vars if they exist 
				// LOGDEBUGF("[Engine] Alloc for script {} with size {}", it->first, sizeof(*it->second));
				// void* p = malloc(sizeof(*it->second));
				// memcpy(p, it->second, sizeof(*it->second));
				obj->AddScript(it->second->Clone());
				obj->GetScript(it->first)->ExposeVars();
				auto& new_reflection_vars = obj->GetScript(it->first)->GetReflection().Get();

				for (auto& [name, v] : new_reflection_vars.str)
					for (auto& [newname, newv] : old_reflection_vars.str)
						if (newname == name)
							*v = newv;
				for (auto& [name, v] : new_reflection_vars.b)
					for (auto& [newname, newv] : old_reflection_vars.b)
						if (newname == name)
							*v = newv;
				for (auto& [name, v] : new_reflection_vars.flt)
					for (auto& [newname, newv] : old_reflection_vars.flt)
						if (newname == name)
							*v = newv;
				for (auto& [name, v] : new_reflection_vars.i8)
					for (auto& [newname, newv] : old_reflection_vars.i8)
						if (newname == name)
							*v = newv;
				for (auto& [name, v] : new_reflection_vars.i16)
					for (auto& [newname, newv] : old_reflection_vars.i16)
						if (newname == name)
							*v = newv;
				for (auto& [name, v] : new_reflection_vars.i32)
					for (auto& [newname, newv] : old_reflection_vars.i32)
						if (newname == name)
							*v = newv;

				LOGDEBUGF("Updated Script {} on Object {}", attached_script_name, obj->name);
			}
		}

		objects_with_scripts.erase(obj->name);
	}

#ifdef _DEBUG
	for (const auto& [obj_name, info] : objects_with_scripts)
	{
		for (const auto& script : info)
		{
			LOGWARNF("Script lost {} on object {}", script.script_name, obj_name);
		}
	}
#endif
}

Engine::TGAME_SCRIPTS& Engine::GetGameScriptsRegister()
{
	return m_gameScripts;
}

std::queue<std::filesystem::path>& Engine::GetDroppedFilesQueue()
{
	return m_droppedFilesQueue;
}

void Engine::SetEngineUI(const FENGINE_UI& p_ui)
{
	m_renderUI = p_ui;
}

void Engine::SetEventCallback(const FEVENT_CALLBACK& callback)
{
	m_eventCallback = callback;
}

void Engine::SetEditorTextureDrawCallback(const FEDITOR_TEXTURE_DRAW_CALLBACK& callback)
{
	m_editorTextureDrawCallback = callback;
}

void Engine::CleanUp()
{
	LOGDEBUG("clean up");

	this->m_isRunning = false;

	LOGDEBUG("shutting down imgui");
	ImGui::SFML::Shutdown();

	LOGDEBUG("closing window");
	m_window.close();
}

void Engine::GameUpdatedWatcher()
{
	std::ifstream f(game_bin_file);
	if (f.is_open())
	{
	}
}

#ifdef _WIN32
LRESULT Engine::WndProc(HWND handle, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (message == WM_DROPFILES)
	{
		HDROP hdrop = reinterpret_cast<HDROP>(wparam);
		POINT drag_point = {0,0};
		DragQueryPoint(hdrop, &drag_point);

		const UINT dropped_files_count = DragQueryFileA(hdrop, 0xFFFFFFFF, nullptr, 0);
		for (UINT i = 0; i < dropped_files_count; i++)
		{
			const UINT buf_size = DragQueryFileA(hdrop, i, nullptr, 0) + 1;
			char* buf = new char[buf_size];

			DragQueryFileA(hdrop, i, buf, buf_size);

			s_Instance->m_droppedFilesQueue.emplace(buf);

			delete[] buf;
		}

		DragFinish(hdrop);
	}

	return CallWindowProcA(reinterpret_cast<WNDPROC>(s_originalWndProc), handle, message, wparam, lparam);
}

#endif

}
