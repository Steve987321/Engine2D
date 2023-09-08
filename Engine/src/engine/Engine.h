#pragma once

#include "EngineMeta.h"
#include "EngineCore.h"

#include <imgui/imgui_internal.h>

namespace Toad
{
	class ENGINE_API Engine
	{
	public:
		using FENGINE_UI = std::function<void(ImGuiContext* ctx)>;

		static Engine& Get();
		static Logger& GetLogger();

		Engine();
		~Engine();

		bool Init();

		void Run();

		// get the window position
		sf::Vector2i GetWindowPos() const;

		sf::Time GetDeltaTime() const;

		Scene& GetScene();
		void SetScene(const Scene& scene);

		void StartGameSession();

		void SetGameScripts();

		void SetEngineUI(const FENGINE_UI& p_ui);

	private:
		bool InitWindow();
		void EventHandler();
		void Render();
		void CleanUp();

	private:
		sf::Time m_deltaTime;
		sf::RenderWindow m_window;
		sf::Clock m_deltaClock;

		FENGINE_UI m_renderUI = nullptr;
		ImGuiIO* m_io = nullptr;

		Scene m_currentScene;

	private:
		std::atomic_bool m_isRunning = false;

		inline static Engine* s_Instance = nullptr;
		inline static Logger s_LoggerInstance;
	};
}