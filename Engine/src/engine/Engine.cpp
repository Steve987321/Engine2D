#include "pch.h"

#include "EngineCore.h"

#include "Engine.h"
#include "Settings.h"

#include "../Game/src/game_core/ScriptRegister.h"

#ifdef __APPLE__
#include <dlfcn.h>
#endif

#include <imgui/imgui.h>
#include <imgui-SFML.h>

namespace Toad
{

Engine::Engine()
{
	s_Instance = this;

	m_currentScene = Scene();
}

Engine::~Engine() = default;

bool Engine::Init(const sf::ContextSettings& settings)
{
	LOGDEBUG("Initializing Engine");

	LoadGameScripts();

	if (!InitWindow(settings))
		return false;

#ifdef TOAD_EDITOR
	LOGDEBUG("Creating window texture for viewport");
	m_windowTexture.create(m_window.getSize().x, m_window.getSize().y);
#endif

	m_isRunning = true;

	return true;
}

void Engine::Run()
{
	LOGDEBUG("starting main loop");

	while (m_window.isOpen())
	{
		// update deltatime
		m_deltaTime = m_deltaClock.restart();

#ifdef TOAD_EDITOR
		// update imgui sfml
		ImGui::SFML::Update(m_window, m_deltaTime);

		// update objects 
		if (m_beginPlay)
			m_currentScene.Update();
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

bool Engine::InitWindow(const sf::ContextSettings& settings)
{
#ifdef TOAD_EDITOR
	LOGDEBUG("Loading editor window");
	m_window.create(sf::VideoMode(1280, 720), "Engine 2D", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize, settings);
	m_window.setFramerateLimit(60);
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
	LOGDEBUG("Loading window");
	m_window.create(sf::VideoMode(600, 600), "Game", sf::Style::Titlebar | sf::Style::Close);
	m_window.setFramerateLimit(60);
	return true;
#endif
}

void Engine::EventHandler()
{
	sf::Event e;
	while (m_window.pollEvent(e))
	{
#ifdef TOAD_EDITOR
		ImGui::SFML::ProcessEvent(e);
#endif
		switch (e.type)
		{

		case sf::Event::Closed:
		{
			LOGDEBUG("closing window");
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
		}

		}
	}
}

void Engine::Render()
{
	m_window.clear(sf::Color::Black); // window bg

	//--------------------draw------------------------//

#ifdef TOAD_EDITOR

	m_windowTexture.clear();

	// Update scene to the texture so it can display on the viewport 
	GetScene().Render(m_windowTexture);
	m_windowTexture.display();

	// imgui
	m_renderUI(ImGui::GetCurrentContext());
	ImGui::SFML::Render(m_window);

#else
	GetScene().Update();
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

sf::RenderWindow& Engine::GetWindow()
{
	return m_window;
}

sf::RenderTexture& Engine::GetWindowTexture()
{
	return m_windowTexture;
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

void Engine::LoadGameScripts()
{
	for (auto& script : m_gameScripts | std::views::values)
	{
		// TODO: save name and give warning about lost scripts 
		script = nullptr;
	}

	std::unordered_map < std::string, ReflectVarsCopy> prev_scripts {};
	for (auto& obj : m_currentScene.objects_map | std::views::values)
	{
		auto& scripts = obj->GetAttachedScripts();
		for (auto i = scripts.begin(); i != scripts.end();)
		{
			ReflectVarsCopy vars;
			i->second->GetReflection().Get().copy(vars);
			prev_scripts[i->first] = vars;
			obj->RemoveScript(i->first);
			i = scripts.begin();
		}
	}

	if (m_currDLL)
		FreeLibrary(m_currDLL);

	// delete old one (if there is one) then rename new one 
	std::string game_dll_path = game_project_directory + game_file_name;
	std::string current_game_dll = game_project_directory + "GameCurrent.dll";

	if (std::ifstream(current_game_dll).good())
	{
		// check if game.dll exists then don't delete 
		if (std::ifstream(game_dll_path).good())
		{
			if (remove(current_game_dll.c_str()) != 0)
			{
				LOGERRORF("Failed to remove file {}", current_game_dll);
			}
		}
	}

	if (!std::ifstream(game_dll_path).good())
	{
		LOGWARNF("Couldn't find game dll file, {}", game_dll_path);
	}
	else
	{
		if (rename(game_dll_path.c_str(), current_game_dll.c_str()) != 0)
		{
			LOGERRORF("Failed to remove file {}", current_game_dll);
			return;
		}
	}

#ifdef _WIN32
	auto dll = LoadLibraryA(current_game_dll.c_str());
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
		for (auto& [attached_script_name, old_reflection_vars] : prev_scripts)
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
	}

#else
	LOGDEBUG("getting game lib");
	auto dll = dlopen(game_bin_path, RTLD_LAZY);
	if (dll == nullptr)
	{
		LOGERRORF("dll is nullptr: {} PATH: {}", dlerror(), game_bin_path);
		return;
	}
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
#endif

#ifdef _DEBUG
	for (const auto& [name, script]: m_gameScripts)
	{
		if (!script)
			LOGWARNF("Script {} is now null", name.c_str());
	}
	for (const auto& name : prev_scripts | std::views::keys)
	{
		LOGWARNF("Script lost: {}", name.c_str());
	}
#endif
}

std::unordered_map<std::string, std::shared_ptr<Script>> & Engine::GetGameScriptsRegister()
{
	return m_gameScripts;
}

void Engine::SetEngineUI(const FENGINE_UI& p_ui)
{
	m_renderUI = p_ui;
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
	std::ifstream f(game_file_name);
	if (f.is_open())
	{
	}
}

}