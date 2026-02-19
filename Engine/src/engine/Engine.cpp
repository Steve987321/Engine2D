#include "pch.h"

#include "engine/Engine.h"
#include "engine/Settings.h"

#include "game_core/ScriptRegister.h"
#include "game_core/Game.h"

#include "engine/PlaySession.h"
#include "engine/AppWindow.h"
#include "engine/systems/Time.h"
#include "engine/systems/Timer.h"
#include "utils/Wrappers.h"
#include "Engine.h"

#ifdef USE_IMGUI
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "implot/implot.h"
#include "imgui-SFML.h"
#endif

#include "engine/render/HDRPipeline.h"
#include "engine/render/fx/FXVignette.h"

namespace Toad
{

static VignetteFX vignette;

using json = nlohmann::ordered_json;

static FENGINE_PRE_UI_CALLBACK pre_ui_callback = nullptr;
static FENGINE_UI render_ui = nullptr;
static FEVENT_CALLBACK event_callback = nullptr;
static FEDITOR_TEXTURE_DRAW_CALLBACK editor_texture_draw_callback = nullptr;
static FONCLOSE_CALLBACK close_callback = nullptr;
static FONDLLCHANGE_CALLBACK on_dll_change_callback = nullptr;

static std::unique_ptr<filewatch::FileWatch<TFILEWATCH_STRTYPE>> dll_file_watch = nullptr;

static Camera editor_cam{"EditorCamera"};
// multiple windows

static std::filesystem::path current_path;
static std::filesystem::path project_path;

static sf::RenderTexture* interacting_texture = nullptr;
static std::unique_ptr<sf::RenderTexture> window_texture = nullptr;

static Camera* interacting_camera = nullptr;

#ifdef USE_IMGUI
static UICtx ui_ctx;
#endif

void Render(AppWindow& window);
void CleanUp();

// finds settings.json and loads them
// void LoadEngineSettings();

static void EventHandler(AppWindow& window)
{
	while (auto e = window.pollEvent())
	{
#ifdef USE_IMGUI
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
			if (IsBeginPlay())
				Input::InvokeKeyPressCallbacks(e->getIf<sf::Event::KeyPressed>()->code);
			break;
		}
		else if (e->is<sf::Event::KeyReleased>())
		{
			if (IsBeginPlay())
				Input::InvokeKeyReleaseCallbacks(e->getIf<sf::Event::KeyReleased>()->code);
			break;
		}

		if (e->is<sf::Event::MouseButtonPressed>())
		{
			if (IsBeginPlay())
				Input::InvokeMousePressCallbacks(e->getIf<sf::Event::MouseButtonPressed>()->button);
			break;
		}
		else if (e->is<sf::Event::MouseButtonReleased>())
		{
			if (IsBeginPlay())
				Input::InvokeMouseReleaseCallbacks(e->getIf<sf::Event::MouseButtonReleased>()->button);
			break;
		}
	}
}

#ifdef TOAD_EDITOR
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
#endif

void SetProjectPath(const std::filesystem::path& path)
{
	LOGDEBUGF("Setting project path: {}", path.string());
	project_path = path;
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

	width = (int) ((float) width * 0.7f);
	height = (int) ((float) height * 0.7f);

	editor_cam.SetPosition({0, 0});
	editor_cam.SetSize({(float) width, (float) height});

	current_path = GetExePath().parent_path();

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
				LOGDEBUGF("[Engine] Failed to parse file: '{}', '{}'", entry.path().string(), e.what());
				continue;
			}

			FSM fsm = FSM::Deserialize(fsm_data);
			std::filesystem::path relative = std::filesystem::relative(entry.path(), current_path);
			ResourceManager::GetFSMs().Add(relative.string(), fsm);
		}
	}
#endif

#ifdef __APPLE__
	Input::AddKeyPressCallback(InputMac::MacKeyPressCallback);
	Input::AddKeyReleaseCallback(InputMac::MacKeyReleaseCallback);
#endif

	// #TODO: change to a .ini or .json
	::AppSettings gsettings;

	DllHandle curr_dll = ScriptManager::GetDLLHandle();

	LOGDEBUGF("curr_dll: {}", (void*) (curr_dll));

	if (curr_dll != nullptr)
	{
		auto get_game_settings = reinterpret_cast<get_game_settings_t*>(DLibGetAddress(curr_dll, "get_game_settings"));
		LOGDEBUGF("DLGetError: {}", DLGetError());
		LOGDEBUGF("get_game_settings: {}", (void*) (get_game_settings));
		if (!get_game_settings)
			LOGERROR("Couldn't get game settings");
		else
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

	LOGDEBUGF("Creating window ({})", gsettings.window_name);

	if (!window.Create(gsettings.window_width, gsettings.window_height, gsettings.frame_limit, gsettings.style, gsettings.window_name))
		return false;
	// window.Create({ (uint32_t)gsettings.window_width, (uint32_t)gsettings.window_height }, "abc", gsettings.style);

#ifdef TOAD_EDITOR
	LOGDEBUG("[Engine] Setting up views & textures");

	bool resize_success = window_texture->resize(window.getSize());
	assert(resize_success && "Failed to resize window texture");
	sf::RenderTexture& editor_cam_texture = GetEditorCameraTexture();
	resize_success = editor_cam_texture.resize(window.getSize());
	assert(resize_success && "Failed to resize editor cam texture");

	editor_cam.OnCreate();
#else
	Screen::SetScreenContentInfoProvider(std::bind(&AppWindow::GetScreenDimensions, &window));

	if (starting_scene)
		Scene::SetScene(starting_scene);
	else
		Scene::SetScene(&Scene::scenes[0]);

	StartGameSession();
#endif

#ifdef USE_IMGUI
	ui_ctx.imgui_ctx = ImGui::GetCurrentContext();
	ui_ctx.implot_ctx = ImPlot::GetCurrentContext();
#endif

    window.setActive(true);

    HDRPipeline::Init(window.getSize().x, window.getSize().y);

	if (vignette.Init())
        HDRPipeline::AddPostFXShader(&vignette);

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
		Mouse::SetRelativeMousePosition(sf::Mouse::getPosition(window));
#endif
		Time::UpdateDeltaTime();

		// handle events
		EventHandler(window);

#ifdef USE_IMGUI
		if (pre_ui_callback)
			pre_ui_callback();

		// update imgui sfml
		ImGui::SFML::Update(window, Time::GetDeltaTimeRaw());
#endif
#ifdef TOAD_EDITOR
		// update objects
		if (IsBeginPlay())
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

	Camera* cam = Camera::GetActiveSceneCamera();

#if defined(TOAD_EDITOR)
	sf::RenderTexture& window_texture = GetWindowTexture();
	sf::RenderTexture& editor_cam_texture = GetEditorCameraTexture();

	window_texture.clear();
	editor_cam_texture.clear();

	Scene::current_scene.Render(window_texture);
	if (cam != nullptr)
	{
		window_texture.setView(cam->GetView());
	}
	window_texture.display();

    window.resetGLStates();
    HDRPipeline::Render(window_texture);

	Scene::current_scene.Render(editor_cam_texture);
	DrawingCanvas::ClearDrawBuffers();

	if (editor_texture_draw_callback)
	{
		editor_texture_draw_callback(editor_cam_texture);
	}
	// sf::RenderTexture t(editor_cam_texture.getSize());
	// vignette.Apply(editor_cam_texture, t);
	// sf::Sprite pt(t.getTexture());
	// editor_cam_texture.draw(pt);

	editor_cam_texture.setView(editor_cam.GetView());
	editor_cam_texture.display();

	// imgui
	render_ui(ImGui::GetCurrentContext());
	AppWindow::UpdateViewports(event_callback);

	for (auto& obj : Scene::current_scene.objects_all)
		for (auto& [name, script] : obj->GetAttachedScripts())
			script->OnImGui(obj.get(), ui_ctx);

	ImGui::SFML::Render(window);
#else
	Scene::current_scene.Render(window);
	DrawingCanvas::ClearDrawBuffers();

#ifdef USE_IMGUI
	for (auto& obj : Scene::current_scene.objects_all)
		for (auto& [name, script] : obj->GetAttachedScripts())
			script->OnImGui(obj.get(), ui_ctx);

	ImGui::SFML::Render(window);
#endif

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
	std::filesystem::path p = project_path;
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
	LOGDEBUGF("Update game binary paths: '{}' '{}'", game_bin_file_name, bin_path);

	if (bin_path.empty() || game_bin_file_name.empty())
		return;

	game_bin_directory = bin_path;
	game_bin_file = game_bin_file_name;

	if (!game_bin_directory.ends_with(PATH_SEPARATOR))
	{
		game_bin_directory += PATH_SEPARATOR;
	}

	if (game_bin_directory.find("bin") == std::string::npos)
		return;

	if (!std::filesystem::exists(game_bin_directory))
		return;

	LOGDEBUG("Setting dll watcher");

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

	LOGDEBUGF("Setting DLL Watcher callback. game bin dir: '{}'", game_bin_directory);
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

#ifdef USE_IMGUI
	LOGDEBUG("shutting down imgui");
	ImGui::SFML::Shutdown();
	ImPlot::DestroyContext();
#endif

	LOGDEBUG("closing window");
	GetWindow().close();

	AppWindow::CleanUpViewports();
}

Camera& GetEditorCamera()
{
	return editor_cam;
}

#ifdef USE_IMGUI

const UICtx& GetUIContext()
{
	return ui_ctx;
}

#endif

} // namespace Toad
