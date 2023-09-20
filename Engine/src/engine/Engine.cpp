#include "pch.h"

#include "EngineCore.h"

#include "engine.h"

#include "Game/src/game_core/ScriptRegister.h"

#include <imgui/imgui.h>
#include <imgui/imgui-SFML.h>

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

	if (!InitWindow(settings)) return false;

	m_windowTexture.create(m_window.getSize().x, m_window.getSize().y);

	m_isRunning = true;

	return true;
}

void Engine::Run()
{
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
		m_currentScene.Update(m_window);
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
	m_window.create(sf::VideoMode(1280, 720), "Engine 2D", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize, settings);
	m_window.setFramerateLimit(60);
	bool res = ImGui::SFML::Init(m_window);
	m_io = &ImGui::GetIO();
	m_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	m_io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	return res;
#else
	// TODO: CHENGE DEEZZ
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
	m_renderUI(m_io->Ctx);
	ImGui::SFML::Render(m_window);

#else
	GetScene().Update(m_window);
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
	auto dll = LoadLibrary(L"Game.dll");
	if (!dll)
	{
		LOGERROR("Couldn't find game dll file, {}", "Game.dll");
		return;
	}

	auto registerScripts = reinterpret_cast<register_scripts_t*>(GetProcAddress(dll, "register_scripts"));
	registerScripts();

	auto getScripts = reinterpret_cast<get_registered_scripts_t*>(GetProcAddress(dll, "get_registered_scripts"));

	// null previous scripts
	for (auto& script : m_gameScripts | std::views::values)
	{
		script = nullptr;
	}

	for (const auto& script : getScripts())
	{
		LOGDEBUG("Load game script: {}", script->GetName().c_str());

		m_gameScripts[script->GetName()] = script;
	}

#ifdef _DEBUG
	for (const auto& [name, script]: m_gameScripts)
	{
		if (!script)
			LOGWARN("Script {} is now null", name.c_str());
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

}