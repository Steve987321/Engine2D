#pragma once

#include "EngineMeta.h"
#include "EngineCore.h"

#include "utils/DLib.h"
#include "filewatch/filewatch.h"
#include <imgui/imgui_internal.h>

struct AppSettings;

namespace Toad
{
	class AppWindow;

#ifdef TOAD_EDITOR
	ENGINE_API void SetInteractingTexture(sf::RenderTexture* texture);
	ENGINE_API Camera* GetInteractingCamera();
	ENGINE_API void SetInteractingCamera(Camera* cam);
#endif

	ENGINE_API const std::filesystem::path& GetCurrentPath();
	ENGINE_API void SetCurrentPath(const std::filesystem::path& path);

	ENGINE_API bool Init();
	ENGINE_API void Run();

	ENGINE_API sf::RenderTexture& GetActiveRenderTexture();
	ENGINE_API sf::RenderTexture& GetEditorCameraTexture();
	ENGINE_API sf::RenderTexture& GetWindowTexture();

	ENGINE_API AppWindow& GetWindow();

	ENGINE_API void UpdateGameBinPaths(std::string_view game_bin_file_name, std::string_view bin_path);
	// load game.dll 
	ENGINE_API void LoadGameScripts();
	// returns a map of scripts with script name as key, some script could be nullptr
	ENGINE_API TGAME_SCRIPTS& GetGameScriptsRegister();
	ENGINE_API Toad::Camera& GetEditorCamera();

	ENGINE_API void SetPreUICallback(const FENGINE_PRE_UI_CALLBACK& callback);
	ENGINE_API void SetEngineUI(const FENGINE_UI& p_ui);
	// use also only for ui
	ENGINE_API void SetEventCallback(const FEVENT_CALLBACK& callback);
	ENGINE_API void SetEditorTextureDrawCallback(const FEDITOR_TEXTURE_DRAW_CALLBACK& callback);
	ENGINE_API void SetOnCloseCallback(const FONCLOSE_CALLBACK& callback);
	ENGINE_API void SetGameDLLWatcherCallback(const FONDLLCHANGE_CALLBACK& callback);
	ENGINE_API void ReleaseGameDLLWatcher();
}
