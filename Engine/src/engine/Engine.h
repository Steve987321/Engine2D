#pragma once

#include "EngineMeta.h"
#include "EngineCore.h"

#include "utils/DLib.h"
#include <imgui/imgui_internal.h>

struct AppSettings;

namespace Toad
{
	/// #TODO: refactor 
	/// #TODO: refactor 
	/// #TODO: refactor 
	/// #TODO: refactor 
	class ENGINE_API Engine
	{
	public:
		using FENGINE_UI = std::function<void(ImGuiContext* ctx)>;
		using FEVENT_CALLBACK = std::function<void(const sf::Event& ctx)>;
		using FEDITOR_TEXTURE_DRAW_CALLBACK = std::function<void(sf::RenderTexture& texture)>;
		using TGAME_SCRIPTS = std::unordered_map<std::string, Script*>;

		Vec2i relative_mouse_pos = {};
#ifdef TOAD_EDITOR
		// editor viewport size
		Vec2i viewport_size = {};
		// camera that is being interacted with
		Camera* interacting_camera = &m_editorCam;
		sf::RenderTexture* interacting_texture = &m_windowTexture;
#endif

		static inline Scene empty_scene;
		static Engine& Get();
		
		Engine();
		~Engine();

		bool Init();

		// Engine main loop
		void Run();

		// get the window position
		sf::Vector2i GetWindowPos() const;

		sf::RenderWindow& GetWindow();

		sf::RenderTexture& GetActiveRenderTexture();
		sf::RenderTexture& GetWindowTexture();
		sf::RenderTexture& GetEditorCameraTexture();

		Camera& GetEditorCamera();

		ImGuiContext* GetImGuiContext();

		bool GameStateIsPlaying() const;
		void StartGameSession();
		void StopGameSession();

		void AddViewport(const sf::VideoMode& mode, std::string_view title, uint32_t style = 0);

		// Returns the world coordinates
		Vec2f ScreenToWorld(const Vec2i& screen_pos);

		void UpdateGameBinPaths(std::string_view game_bin_file_name, std::string_view bin_path);
		// load game.dll 
		void LoadGameScripts();
		// returns a map of scripts with script name as key, some script could be nullptr
		TGAME_SCRIPTS& GetGameScriptsRegister();

		std::queue<std::filesystem::path>& GetDroppedFilesQueue();

		void SetEngineUI(const FENGINE_UI& p_ui);
		// use also only for ui
		void SetEventCallback(const FEVENT_CALLBACK& callback);
		void SetEditorTextureDrawCallback(const FEDITOR_TEXTURE_DRAW_CALLBACK& callback);

	private:
		bool InitWindow(const AppSettings& settings);
		void EventHandler();
		void Render();
		void CleanUp();

		// checks to see if a new game.dll is available 
		void GameUpdatedWatcher();

#ifdef _WIN32
		static LRESULT CALLBACK WndProc(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);
#endif

	private:
		float m_tickps = 1000.f / 50.f;
		int m_width = 500, m_height = 500;

		sf::RenderWindow m_window;

		sf::RenderTexture m_windowTexture;
		sf::RenderTexture m_windowEditorCamTexture;

		FENGINE_UI m_renderUI = nullptr;
		FEVENT_CALLBACK m_eventCallback = nullptr;
		FEDITOR_TEXTURE_DRAW_CALLBACK m_editorTextureDrawCallback = nullptr;
		ImGuiIO* m_io = nullptr;

		// instances of all game scripts
		TGAME_SCRIPTS m_gameScripts = {};

		std::queue<std::filesystem::path> m_droppedFilesQueue;

		bool m_beginPlay = false, m_beginPlayPrev = false;
		Camera m_editorCam {"EditorCamera"};

		// multiple windows
		std::vector<std::shared_ptr<sf::RenderWindow>> m_viewports;
		// mouse position relative to window/viewport 

        DllHandle m_currDLL{};
#ifdef _WIN32
		inline static LONG_PTR s_originalWndProc = NULL;
#endif

	private:
		std::atomic_bool m_isRunning = false;

		inline static Engine* s_Instance = nullptr;

		std::filesystem::path m_currentPath;
	};
}