#pragma once

#include "EngineMeta.h"
#include "EngineCore.h"

#include <imgui/imgui_internal.h>

struct AppSettings;

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

		// Engine main loop
		void Run();

		// get the window position
		sf::Vector2i GetWindowPos() const;

		sf::Time GetDeltaTime() const;
		sf::Time GetFixedDeltaTime() const;

		sf::RenderWindow& GetWindow();

		sf::RenderTexture& GetWindowTexture();

		ResourceManager& GetResourceManager();

		Scene& GetScene();
		void SetScene(const Scene& scene);

		ImGuiContext* GetImGuiContext();

		bool GameStateIsPlaying() const;
		void StartGameSession();
		void StopGameSession();

		void UpdateGameBinPaths(std::string_view game_bin_file_name, std::string_view bin_path);
		// load game.dll 
		void LoadGameScripts();
		// returns a map of scripts with script name as key, some script could be nullptr
		std::unordered_map<std::string, std::shared_ptr<Script>>& GetGameScriptsRegister();

		std::queue<std::filesystem::path>& GetDroppedFilesQueue();

		void SetEngineUI(const FENGINE_UI& p_ui);

		static constexpr int fixed_update_frequency = 1000 / 50;

	private:
		bool InitWindow(const AppSettings& settings);
		void EventHandler();
		void Render();
		void CleanUp();

		// checks to see if a new game.dll is available 
		void GameUpdatedWatcher();

		static LRESULT CALLBACK WndProc(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);

	private:
		float m_tickps = 1000.f / 50.f;
		sf::Time m_deltaTime;
		sf::Time m_fixedDeltaTime;

		sf::RenderWindow m_window;
		sf::Clock m_deltaClock;

		sf::RenderTexture m_windowTexture;

		FENGINE_UI m_renderUI = nullptr;
		ImGuiIO* m_io = nullptr;

		Scene m_currentScene = {};
		std::vector<Scene> m_scenes = {};

		ResourceManager m_resourceManager;

		// instances of all game scripts
		std::unordered_map<std::string, std::shared_ptr<Script>> m_gameScripts = {};

		std::queue<std::filesystem::path> m_droppedFilesQueue;

		bool m_beginPlay = false, m_beginPlayPrev = false;

#ifdef _WIN32
        HMODULE m_currDLL{};
		inline static LONG_PTR s_originalWndProc = NULL;
#else
        void* m_currDLL{};
#endif

	private:
		std::atomic_bool m_isRunning = false;

		inline static Engine* s_Instance = nullptr;
		inline static Logger s_LoggerInstance;

		std::filesystem::path m_current_path;
	};
}