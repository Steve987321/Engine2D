#include "pch.h"

#include "EngineCore.h"

#include "engine.h"

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
	log_Debug("Initializing Engine");

	if (!InitWindow(settings)) return false;

	m_isRunning = true;

	return true;
}

void Engine::Run()
{
	while (m_window.isOpen())
	{
		// update deltatime
		m_deltaTime = m_deltaClock.restart();

		// update imgui sfml
		ImGui::SFML::Update(m_window, m_deltaTime);

		// handle events 
		EventHandler();

		if (m_beginPlay)
		{
			// update scene objects
			m_currentScene.Update(m_window);
		}

		// render the window and contents
		Render();
	}

	CleanUp();
}

bool Engine::InitWindow(const sf::ContextSettings& settings)
{
#ifdef TOAD_EDITOR
	m_window.create(sf::VideoMode(600, 600), "Engine 2D", sf::Style::Titlebar | sf::Style::Close, settings);
	m_window.setFramerateLimit(60);
	bool res = ImGui::SFML::Init(m_window);
	m_io = &ImGui::GetIO();
	m_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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
			log_Ok("closing window");
			m_window.close();
			break;
		}

		}
	}
}

void Engine::Render()
{
#ifdef TOAD_EDITOR
	// show ui
	m_renderUI(m_io->Ctx);
#endif
	m_window.clear(sf::Color::Black); // window bg

	//--------------------draw------------------------//
#ifdef TOAD_EDITOR
// imgui
	ImGui::SFML::Render(m_window);
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

void Engine::StartGameSession()
{
	m_beginPlay = true;
	m_currentScene.Start();
}

void Engine::StopGameSession()
{
	m_beginPlay = false;
}

void Engine::SetGameScripts()
{
}

void Engine::SetEngineUI(const FENGINE_UI& p_ui)
{
	m_renderUI = p_ui;
}

void Engine::CleanUp()
{
	log_Debug("clean up");

	this->m_isRunning = false;

	log_Debug("shutting down imgui");
	ImGui::SFML::Shutdown();

	log_Debug("closing window");
	m_window.close();
}

}