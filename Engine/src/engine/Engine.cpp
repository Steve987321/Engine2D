#include "pch.h"

#ifndef NDEBUG
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "engine/Engine.h"
#include "engine/Settings.h"

#include "game_core/ScriptRegister.h"
#include "game_core/Game.h"

#include "engine/PlaySession.h"
#include "engine/AppWindow.h"
#include "engine/systems/Time.h"
#include "engine/systems/Timer.h"
#include "utils/Wrappers.h"
#include <imgui/imgui.h>
#include "imgui-SFML.h"

namespace Toad
{

using json = nlohmann::ordered_json;

static FENGINE_PRE_UI_CALLBACK pre_ui_callback = nullptr;
static FENGINE_UI render_ui = nullptr;
static FEVENT_CALLBACK event_callback = nullptr;
static FEDITOR_TEXTURE_DRAW_CALLBACK editor_texture_draw_callback = nullptr;
static FONCLOSE_CALLBACK close_callback = nullptr;
static FONDLLCHANGE_CALLBACK on_dll_change_callback = nullptr;

static std::unique_ptr<filewatch::FileWatch<TFILEWATCH_STRTYPE>> dll_file_watch = nullptr;

static Camera editor_cam{ "EditorCamera" };
// multiple windows

static DllHandle curr_dll{};

static std::filesystem::path current_path;

static sf::RenderTexture* interacting_texture = nullptr;
static std::unique_ptr<sf::RenderTexture> window_texture = nullptr;

static sf::Texture default_texture;

static Camera* interacting_camera = nullptr;

void Render(AppWindow& window);
void CleanUp();

namespace DrawingCanvas
{
extern void DrawBuffers(sf::RenderTarget& target);
}

// finds settings.json and loads them 
void LoadEngineSettings();

static void EventHandler(AppWindow& window)
{
    while (auto e = window.pollEvent())
	{
#if defined(TOAD_EDITOR) || !defined(NDEBUG)
		ImGui::SFML::ProcessEvent(window, *e);
#endif
#if defined(TOAD_EDITOR)
		event_callback(*e);
#endif
		if (e->is<sf::Event::Closed>())
		{
			StopGameSession();
			window.close();
			break;
		}

#ifdef TOAD_EDITOR
		if (e->is<sf::Event::Resized>())
		{
			// update window texture size to new window size
			bool resize_success = GetWindowTexture().resize(window.getSize());
			assert(resize_success && "Failed to resize window texture to new resized size");
			break;
		}
#endif

		if (e->is<sf::Event::KeyPressed>())
		{
			if (begin_play)
				Input::InvokeKeyPressCallbacks(e->getIf<sf::Event::KeyPressed>()->code);
			break;
		}

		if (e->is<sf::Event::KeyReleased>())
		{
			if (begin_play)
				Input::InvokeKeyReleaseCallbacks(e->getIf<sf::Event::KeyReleased>()->code);
			break;
		}

		if (e->is<sf::Event::MouseButtonPressed>())
		{
			if (begin_play)
				Input::InvokeMousePressCallbacks(e->getIf<sf::Event::MouseButtonPressed>()->button);
			break;
		}
        
		if (e->is<sf::Event::MouseButtonReleased>())
		{
			if (begin_play)
				Input::InvokeMouseReleaseCallbacks(e->getIf<sf::Event::MouseButtonReleased>()->button);
			break;
		}
	}
}

void SetInteractingTexture(sf::RenderTexture* texture)
{
	interacting_texture = texture;
}

Toad::Camera* GetInteractingCamera()
{
	return interacting_camera;
}

void SetInteractingCamera(Camera* cam)
{
	interacting_camera = cam;
}

const std::filesystem::path& GetCurrentPath()
{
	return current_path;
}

void SetCurrentPath(const std::filesystem::path& path)
{
	current_path = path;
}

bool Init()
{
	LOGDEBUG("[Engine] Initializing Engine");

	window_texture = std::make_unique<sf::RenderTexture>();
	interacting_camera = &editor_cam;
	interacting_texture = window_texture.get();

	int width = 500;
	int height = 500;
	GetDesktopDimensions(width, height);

	width = (int)((float)width * 0.7f);
	height = (int)((float)height * 0.7f);

	editor_cam.SetPosition({ 0, 0 });
	editor_cam.SetSize({ (float)width, (float)height });

	current_path = get_exe_path().parent_path();
	
	ResourceManager::Init();

	for (const auto& e : std::filesystem::recursive_directory_iterator(current_path))
	{
		if (e.path().filename().string().find("Game") != std::string::npos && e.path().extension() == LIB_FILE_EXT)
		{
			UpdateGameBinPaths(e.path().filename().string(), e.path().parent_path().string());
		}
	}

	LoadGameScripts();

#ifndef TOAD_EDITOR
	// load settings
	for (const auto& e : std::filesystem::directory_iterator(current_path))
	{
		if (e.path().filename() == "settings" && e.path().extension() == ".json")
		{
			
		}
	}

	Scene* starting_scene = nullptr;

	// get start scene and get game dl
	for (const auto& entry : std::filesystem::recursive_directory_iterator(current_path))
	{
		if (entry.path().filename().string().find("Game") != std::string::npos && entry.path().extension() == LIB_FILE_EXT)
		{
			UpdateGameBinPaths(entry.path().filename().string(), entry.path().parent_path().string());
		}

		if (!entry.path().has_extension())
			continue;

		if (entry.path().extension() == FILE_EXT_TOADSCENE)
		{
			Scene& s = LoadScene(entry.path());

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
		if (entry.path().extension() == FILE_EXT_FSM)
		{
			std::ifstream f(entry.path());
			json fsm_data;
			try
			{
				fsm_data = json::parse(f);
			}
			catch (std::exception& e)
			{
				LOGDEBUGF("[Engine] Failed to parse {}", entry.path().string());
				continue;
			}

			FSM fsm = FSM::Deserialize(fsm_data);
			std::filesystem::path relative = std::filesystem::relative(entry.path(), current_path);
			ResourceManager::GetFSMs().Add(relative.string(), fsm);
		}
	}
#endif

	// #TODO: change to a .ini or .json
	::AppSettings gsettings;
	if (curr_dll != nullptr)
	{
        auto get_game_settings = reinterpret_cast<get_game_settings_t*>(DLibGetAddress(curr_dll, "get_game_settings"));
		gsettings = get_game_settings();
	}

#ifdef TOAD_EDITOR
	gsettings.frame_limit = 30;
	gsettings.style = sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize;
	gsettings.window_width = width;
	gsettings.window_height = height;
	gsettings.window_name = "Engine 2D";
#endif 
	AppWindow& window = GetWindow();

	if (!window.Create(gsettings.window_width, gsettings.window_height, gsettings.frame_limit, gsettings.style, gsettings.window_name))
		return false;
	//window.Create({ (uint32_t)gsettings.window_width, (uint32_t)gsettings.window_height }, "abc", gsettings.style);

#ifdef TOAD_EDITOR
	LOGDEBUG("[Engine] Setting up views & textures");

	bool resize_success = window_texture->resize(window.getSize());
	assert(resize_success && "Failed to resize window texture");
	sf::RenderTexture& editor_cam_texture = GetEditorCameraTexture();
	resize_success = editor_cam_texture.resize(window.getSize());
	assert(resize_success && "Failed to resize editor cam texture");

	editor_cam.OnCreate();
#else
	begin_play = true;
	if (starting_scene)
		Scene::SetScene(starting_scene);
	else
		Scene::SetScene(&Scene::scenes[0]);
#endif

	return true;
}

void Run()
{
	AppWindow& window = GetWindow();

	while (window.isOpen())
	{ 
#ifdef TOAD_EDITOR
		if (!window.hasFocus())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
		}
#else 
		Mouse::relative_mouse_pos = sf::Mouse::getPosition(window);
#endif
		Time::UpdateDeltaTime();

		// handle events 
		EventHandler(window);

#if defined(TOAD_EDITOR) || !defined(NDEBUG)
		if (pre_ui_callback)
			pre_ui_callback();

		// update imgui sfml
		ImGui::SFML::Update(window, Time::GetDeltaTimeRaw());
#endif 
#ifdef TOAD_EDITOR
		// update objects 
		if (begin_play)
		{
			Scene::current_scene.Update();
		}
#else
		Scene::current_scene.Update();
#endif

		// render the window and contents
		Render(window);
	}

#ifndef TOAD_EDITOR
	Scene::current_scene.End(nullptr);
#endif

	CleanUp();
}

void Render(AppWindow& window)
{
	window.clear(sf::Color::Black); // window bg

	Camera* cam = Camera::GetActiveCamera();

#if defined(TOAD_EDITOR)
	sf::RenderTexture& window_texture = GetWindowTexture();
	sf::RenderTexture& editor_cam_texture = GetEditorCameraTexture();

	window_texture.clear();
	editor_cam_texture.clear();

	// Update scene to the texture so it can display on the (game) viewport 
	Scene::current_scene.Render(window_texture);
	DrawingCanvas::DrawVertices(window_texture);
    DrawingCanvas::DrawBuffers(window_texture);
	if (cam != nullptr)
	{
		window_texture.setView(cam->GetView());
	}
	window_texture.display();

	Scene::current_scene.Render(editor_cam_texture);
	DrawingCanvas::DrawVertices(editor_cam_texture);
    DrawingCanvas::DrawBuffers(editor_cam_texture);
    DrawingCanvas::ClearDrawBuffers();

	if (editor_texture_draw_callback)
	{
		editor_texture_draw_callback(editor_cam_texture);
	}

	editor_cam_texture.setView(editor_cam.GetView());
	editor_cam_texture.display();

	// imgui
	render_ui(ImGui::GetCurrentContext());

	for (auto& obj : Scene::current_scene.objects_all)
		for (auto& [name, script] : obj->GetAttachedScripts())
			script->OnImGui(obj.get(), ImGui::GetCurrentContext());

	ImGui::SFML::Render(window);
	AppWindow::UpdateViewports(event_callback);
#else
#ifndef NDEBUG
	for (auto& obj : Scene::current_scene.objects_all)
		for (auto& [name, script] : obj->GetAttachedScripts())
			script->OnImGui(obj.get(), ImGui::GetCurrentContext());

	ImGui::SFML::Render(window);
#endif 
	Scene::current_scene.Render(window);

	if (cam != nullptr)
	{
		window.setView(cam->GetView());
	}
#endif

	window.display();
}

sf::RenderTexture& GetActiveRenderTexture()
{
#ifdef TOAD_EDITOR
	return *interacting_texture;
#else
	return GetWindowTexture(); 
#endif 
}

sf::RenderTexture& GetEditorCameraTexture()
{
	static sf::RenderTexture editor_cam_texture;
	return editor_cam_texture;
}

sf::RenderTexture& GetWindowTexture()
{
	return *window_texture;
}

AppWindow& GetWindow()
{
	static AppWindow window;
	return window;
}

std::filesystem::path GetAssetPath()
{
    // use game bin directory
#ifdef TOAD_EDITOR
    std::filesystem::path p = game_bin_directory;
    p = p.parent_path();
    for (auto& e : std::filesystem::directory_iterator(p))
    {
        if (e.path().filename().string().find("Game") != std::string::npos)
        {
            p = e.path() / "src" / "assets";
            assert(std::filesystem::exists(p) && "GetAssetPath() returned invalid path");
            return p;
        }
    }
#else
    return GetCurrentPath();
#endif
    
    return {};
}

void UpdateGameBinPaths(std::string_view game_bin_file_name, std::string_view bin_path)
{
    game_bin_directory = bin_path;
    game_bin_file = game_bin_file_name;
    
    if (!game_bin_directory.ends_with(PATH_SEPARATOR))
    {
        game_bin_directory += PATH_SEPARATOR;
    }
    
    if (game_bin_directory.find("bin") == std::string::npos)
        return;
    
#ifdef _WIN32
    std::wstring ws(game_bin_directory.size(), ' ');
    ws.resize(std::mbstowcs(&ws[0], game_bin_directory.c_str(), game_bin_directory.size()));
    
    dll_file_watch = std::make_unique<filewatch::FileWatch<TFILEWATCH_STRTYPE>>(ws, on_dll_change_callback);
#else
    dll_file_watch = std::make_unique<filewatch::FileWatch<TFILEWATCH_STRTYPE>>(game_bin_directory, on_dll_change_callback);
#endif
}

void LoadGameScripts()
{
	auto& f = on_dll_change_callback;
	ReleaseGameDLLWatcher();
	
	ScriptManager::LoadScripts();

	if (f)
		SetGameDLLWatcherCallback(f);
}

TGAME_SCRIPTS& GetGameScriptsRegister()
{
	return ScriptManager::GetScripts();
}

void SetPreUICallback(const FENGINE_PRE_UI_CALLBACK& callback)
{
	pre_ui_callback = callback;
}

void SetEngineUI(const FENGINE_UI& p_ui)
{
	render_ui = p_ui;
}

void SetEventCallback(const FEVENT_CALLBACK& callback)
{
	event_callback = callback;
}

void SetEditorTextureDrawCallback(const FEDITOR_TEXTURE_DRAW_CALLBACK& callback)
{
	editor_texture_draw_callback = callback;
}

void SetOnCloseCallback(const FONCLOSE_CALLBACK& callback)
{
	close_callback = callback;
}

void SetGameDLLWatcherCallback(const FONDLLCHANGE_CALLBACK& callback)
{
	on_dll_change_callback = callback;

	if (!std::filesystem::exists(game_bin_directory))
	{
		LOGERRORF("Can't add game dll watcher to non existing file: {}", game_bin_directory);
		return;
	}
    
    if (game_bin_directory.find("bin") == std::string::npos)
        return;

#ifdef _WIN32
    std::wstring ws(game_bin_directory.size(), ' ');
    ws.resize(std::mbstowcs(&ws[0], game_bin_directory.c_str(), game_bin_directory.size()));
    
    dll_file_watch = std::make_unique<filewatch::FileWatch<TFILEWATCH_STRTYPE>>(ws, on_dll_change_callback);
#else
    dll_file_watch = std::make_unique<filewatch::FileWatch<TFILEWATCH_STRTYPE>>(game_bin_directory, on_dll_change_callback);
#endif
    
	LOGDEBUG("Dll watch {}", game_bin_directory.string());
}

void ReleaseGameDLLWatcher()
{
	LOGDEBUG("Releasing dll watcher");
	dll_file_watch = nullptr;
}

void CleanUp()
{
	LOGDEBUG("clean up");

	if (close_callback)
		close_callback(0);

	window_texture.release();

	LOGDEBUG("shutting down imgui");
	ImGui::SFML::Shutdown();

	LOGDEBUG("closing window");
	GetWindow().close();
}

Camera& GetEditorCamera()
{
	return editor_cam;
}

}
