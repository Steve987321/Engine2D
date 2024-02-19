#include "pch.h"

#ifdef _DEBUG
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "Engine.h"
#include "Settings.h"

#include "game_core/ScriptRegister.h"
#include "game_core/Game.h"

#include "engine/systems/Timer.h"

#include <imgui/imgui.h>
#include <imgui-SFML.h>

namespace Toad
{

Engine::Engine()
{
	s_Instance = this;

	m_currentScene = Scene();

	m_editorCam.SetPosition({ 0, 0 });
	m_editorCam.SetSize({ 1280, 720});
}

Engine::~Engine() = default;

bool Engine::Init()
{
	LOGDEBUG("Initializing Engine");

	for (const auto& e : std::filesystem::recursive_directory_iterator(std::filesystem::current_path()))
	{
		if (e.path().filename().string().find("Game") != std::string::npos && e.path().extension() == LIB_FILE_EXT)
		{
			UpdateGameBinPaths(e.path().filename().string(), e.path().parent_path().string());
		}
	}

	m_current_path = std::filesystem::current_path();
	LoadGameScripts();

#ifndef TOAD_EDITOR
	std::vector<Scene> found_scenes;

	LoadGameScripts();

	for (const auto& e : std::filesystem::recursive_directory_iterator(std::filesystem::current_path()))
	{
		if (e.path().filename().string().find("Game") != std::string::npos && e.path().extension() == LIB_FILE_EXT)
		{
			UpdateGameBinPaths(e.path().filename().string(), e.path().parent_path().string());
		}

		if (e.path().has_extension() && e.path().extension() == ".TSCENE")
		{
			Scene s = LoadScene(e.path());

			std::string lower;
			for (char c : s.name)
			{
				lower += std::tolower(c);
			}

			if (lower.find("start") != std::string::npos)
			{
				m_currentScene = s;
			}

			if (!s.objects_map.empty())
			{
				found_scenes.push_back(s);
			}
		}
	}

	m_scenes.reserve(found_scenes.size());
	for (const Scene& s : found_scenes)
	{
		m_scenes.push_back(s);
	}

#endif

	AppSettings gsettings;
	if (m_currDLL != nullptr)
	{

#ifdef _WIN32
		auto get_game_settings = reinterpret_cast<get_game_settings_t*>(GetProcAddress(m_currDLL, "get_game_settings"));
#else
        auto get_game_settings = reinterpret_cast<get_game_settings_t*>(dlsym(m_currDLL, "get_game_settings"));
#endif
		gsettings = get_game_settings();
	}

	if (!InitWindow(gsettings))
		return false;
	 
#ifdef TOAD_EDITOR
	LOGDEBUG("Creating window texture for viewport & game view");
	m_windowTexture.create(m_window.getSize().x, m_window.getSize().y);
	m_windowEditorCamTexture.create(m_window.getSize().x, m_window.getSize().y);
#endif

	m_isRunning = true;

	return true;
}

void Engine::Run()
{
#ifndef TOAD_EDITOR
	m_beginPlay = true;
	SetScene(m_currentScene);
#endif

	while (m_window.isOpen())
	{
		// update deltatime
		m_deltaTime = m_deltaClock.restart();

#ifdef TOAD_EDITOR
		// update imgui sfml
		ImGui::SFML::Update(m_window, m_deltaTime);

		// update objects 
		if (m_beginPlay)
		{
			m_currentScene.Update();
		}
#else
		m_currentScene.Update();
#endif

		// handle events 
		EventHandler();
		// render the window and contents
		Render();
	}

	CleanUp();
}

bool Engine::InitWindow(const AppSettings& settings)
{
#ifdef TOAD_EDITOR
	LOGDEBUG("Loading editor window");
	m_window.create(sf::VideoMode(1280, 720), "Engine 2D", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize, sf::ContextSettings());
	m_window.setFramerateLimit(60);

#ifdef _WIN32
	HWND window_handle = m_window.getSystemHandle();
    DragAcceptFiles(window_handle, TRUE);
	s_originalWndProc = SetWindowLongPtrA(window_handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));
#else
    void* window_handle = m_window.getSystemHandle();
#endif

	bool res = ImGui::SFML::Init(m_window, false);
	LOGDEBUGF("ImGui SFML Init result: {}", res);
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
	LOGDEBUG("Creating window");
	m_window.create(sf::VideoMode(settings.window_width, settings.window_height), settings.window_name, sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize, settings.ctx_settings);
	m_window.setFramerateLimit(settings.frame_limit);
	return true;
#endif
}

void Engine::EventHandler()
{
	sf::Event e;
	while (m_window.pollEvent(e))
	{
#ifdef TOAD_EDITOR
		ImGui::SFML::ProcessEvent(m_window, e);
		m_eventCallback(e);
#endif

		switch (e.type)
		{
		
		case sf::Event::Closed:
		{
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
			break;
		}

		case sf::Event::KeyReleased:
		{
			break;
		}

		case sf::Event::MouseButtonPressed:
		{
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

#if defined(_DEBUG) && !defined(TOAD_EDITOR)
	ImDrawList* draw = ImGui::GetWindowDrawList();
	const char* err_msg = "NO CAMERA'S IN SCENE ARE AVAILABLE FOR RENDERING";
	ImVec2 size = ImGui::CalcTextSize(err_msg) / 2;
	draw->AddText({ m_window.getSize().x / 2 - size.x, m_window.getSize().y / 2 - size.y }, IM_COL32(255, 0, 0, 255), err_msg);
#endif

#if defined(TOAD_EDITOR)
	m_windowTexture.clear();
	m_windowEditorCamTexture.clear();

	// Update scene to the texture so it can display on the (game) viewport 
	GetScene().Render(m_windowTexture);
	if (cam != nullptr)
	{
		m_windowTexture.setView(cam->GetView());
	}
	m_windowTexture.display();

	GetScene().Render(m_windowEditorCamTexture);

	if (m_editorTextureDrawCallback)
	{
		m_editorTextureDrawCallback(m_windowEditorCamTexture);
	}

	m_windowEditorCamTexture.setView(m_editorCam.GetView());
	m_windowEditorCamTexture.display();

	// imgui
	m_renderUI(ImGui::GetCurrentContext());

	ImGui::SFML::Render(m_window);

	auto it = m_viewports.begin();
	while (it != m_viewports.end())
	{
		bool erased = false;
		ImGui::SFML::Update(*(*it), m_deltaTime);

		sf::Event e2;
		while (!erased && (*it)->pollEvent(e2))
		{
			ImGui::SFML::ProcessEvent(*(*it), e2);
			m_eventCallback(e2);

			switch (e2.type)
			{
			case e2.Closed:
				(*it)->close();
				it = m_viewports.erase(it);
				erased = true;
				break;
			}
		}

		if (!erased)
		{
			++it;
		}
	}

	for (const auto& viewport : m_viewports)
	{
		viewport->clear(sf::Color::Black);
		ImGui::Begin("abc");
		ImGui::Text("ABC");

		ImGui::End();
		ImGui::SFML::Render(*viewport);
		viewport->display();
	}
#else
	GetScene().Render(m_window);

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

Logger& Engine::GetLogger()
{
	return s_LoggerInstance;
}

sf::Time Engine::GetDeltaTime() const
{
	return m_deltaTime;
}

sf::Time Engine::GetFixedDeltaTime() const
{
	return m_fixedDeltaTime;
}

sf::RenderWindow& Engine::GetWindow()
{
	return m_window;
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

ResourceManager& Engine::GetResourceManager()
{
	return m_resourceManager;
}

Scene& Engine::GetScene()
{
	return m_currentScene;
}

void Engine::SetScene(const Scene& scene)
{
	m_currentScene = scene;

	if (m_beginPlay)
		m_currentScene.Start();
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
	m_beginPlay = true;
	m_currentScene.Start();
}

void Engine::StopGameSession()
{
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
	for (auto& script : m_gameScripts | std::views::values)
	{
		// TODO: save name and give warning about lost scripts 
		script = nullptr;
	}
	using object_script = struct { std::string script_name; ReflectVarsCopy reflection; };
	std::unordered_map <std::string, std::vector<object_script >> objects_with_scripts{};

	for (auto& obj : m_currentScene.objects_map | std::views::values)
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
			objects_with_scripts[obj->name].emplace_back(object_script{i->first, vars});

			obj->RemoveScript(i->first);
			i = scripts.begin();
		}
	}

	if (m_currDLL)
	{
#ifdef _WIN32
		FreeLibrary(m_currDLL);
#else
        dlclose(m_currDLL);
#endif
	}

	fs::path game_dll_path = game_bin_directory + game_bin_file;
#ifdef _WIN32
	fs::path current_game_dll = game_bin_directory + "GameCurrent.dll";
#else
//    fs::path current_game_dll = game_bin_directory + "libGameCurrent.dylib";
    fs::path current_game_dll = game_bin_directory + game_bin_file;
#endif

#if defined TOAD_EDITOR && _WIN32
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

#ifdef _WIN32
//#ifdef __APPLE__
//    if (!game_bin_directory.empty())
//    {
//#endif
        if (!fs::exists(game_dll_path)) {
            LOGWARNF("Couldn't find game dll file, {}", game_dll_path);
        } else {
            try {
                fs::rename(game_dll_path, current_game_dll);
            }
            catch (fs::filesystem_error &e) {
                LOGERRORF("{}", e.what());
                return;
            }
        }
//#ifdef __APPLE__
//    }
#endif
//#endif

#ifdef _WIN32
	auto dll = LoadLibraryA(current_game_dll.string().c_str());
	if (!dll)
	{
		LOGERRORF("Couldn't load game dll file, {}", current_game_dll);
		return;
	}

	m_currDLL = dll;

	auto registerScripts = reinterpret_cast<register_scripts_t*>(GetProcAddress(dll, "register_scripts"));
	auto getScripts = reinterpret_cast<get_registered_scripts_t*>(GetProcAddress(dll, "get_registered_scripts"));

	registerScripts();

	for (const auto& script : getScripts())
	{
		LOGDEBUGF("Load game script: {}", script->GetName().c_str());

		m_gameScripts[script->GetName()] = script;
	}

	// update scripts on object if it has an old version
	for (auto& [obj_name, obj] : m_currentScene.objects_map)
	{
		if (!objects_with_scripts.contains(obj_name))
		{
			continue;
		}

		const auto& prev_obj_state = objects_with_scripts[obj_name];

		for (auto& [attached_script_name, old_reflection_vars] : prev_obj_state)
		{
			auto it = m_gameScripts.find(attached_script_name);
			if (it != m_gameScripts.end())
			{
				// update exposed vars if they exist 
				obj->AddScript(it->second->Clone());

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

				LOGDEBUGF("Updated Script {} on Object {}", attached_script_name, obj_name);
			}
		}

		objects_with_scripts.erase(obj_name);
	}

#else
	LOGDEBUG("getting game lib");
	auto dll = dlopen(current_game_dll.c_str(), RTLD_LOCAL | RTLD_NOW);
	if (dll == nullptr)
	{
		LOGERRORF("dll is nullptr: {} PATH: {}", dlerror(), current_game_dll);
		return;
	}

    m_currDLL = dll;
	LOGERROR("found dll");

	auto registerScripts = reinterpret_cast<register_scripts_t*>(dlsym(dll, "register_scripts"));
	LOGDEBUGF("registerScripts: {}", (void*)registerScripts);
	if (!registerScripts)
		return;
	registerScripts();

	auto getScripts = reinterpret_cast<get_registered_scripts_t*>(dlsym(dll, "get_registered_scripts"));
	LOGDEBUGF("getScripts: {}", (void*)getScripts);
	if (getScripts == nullptr)
		return;

	// null previous scripts
	for (auto& script : m_gameScripts | std::views::values)
	{
		script = nullptr;
	}

	for (const auto& script : getScripts())
	{
		LOGDEBUGF("Load game script: {}", script->GetName().c_str());

		m_gameScripts[script->GetName()] = script;
	}

    // update scripts on object if it has an old version
    for (auto& [obj_name, obj] : m_currentScene.objects_map)
    {
        if (!objects_with_scripts.contains(obj_name))
        {
            continue;
        }

        const auto& prev_obj_state = objects_with_scripts[obj_name];

        for (auto& [attached_script_name, old_reflection_vars] : prev_obj_state)
        {
            auto it = m_gameScripts.find(attached_script_name);
            if (it != m_gameScripts.end())
            {
                // update exposed vars if they exist
                obj->AddScript(it->second->Clone());

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

                LOGDEBUGF("Updated Script {} on Object {}", attached_script_name, obj_name);
            }
        }

        objects_with_scripts.erase(obj_name);
    }
#endif

#ifdef _DEBUG
	for (const auto& [name, script] : m_gameScripts)
	{
		if (!script)
			LOGWARNF("Script {} is now null", name.c_str());
	}
	for (const auto& [obj_name, info] : objects_with_scripts)
	{
		for (const auto& script : info)
		{
			LOGWARNF("Script lost {} on object {}", script.script_name, obj_name);
		}
	}
#endif
}

std::unordered_map<std::string, std::shared_ptr<Script>> & Engine::GetGameScriptsRegister()
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
