#include "pch.h"

#ifdef TOAD_EDITOR
#include <algorithm>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "engine/Engine.h"

#include "UI.h"

#include "imgui-SFML.h"
#include "ui/FileBrowser.h"
#include "ui/AnimationEditor.h"
#include "ui/GameAssetsBrowser.h"
#include "ui/FSMGraphEditor.h"
#include "ui/TextEditor.h"
#include "ui/MessageQueue.h"
#include "ui/Inspector.h"
#include "ui/SceneHierarchy.h"
#include "ui/Viewport.h"

#include "engine/systems/build/Package.h"
#include "project/ToadProject.h"
#include "SceneHistory.h"
#include "utils/FileDialog.h"
#include "engine/utils/Helpers.h"
#include "engine/systems/Animation.h"
#include "engine/Types.h"
 
using json = nlohmann::ordered_json;

using namespace Toad;

static SceneHistory scene_history{};
static ui::GameAssetsBrowser* browser = nullptr;
static bool is_scene_loaded = false;
static bool load_ini = true; 
static bool load_ini_file = true;
static const char* ini_buf = nullptr;

// set to true if we should reload scritps
static bool should_reload = false;

// get installed directory (distro)
// get engine solution directory (source)
extern std::filesystem::path GetEngineDirectory();
extern std::filesystem::path GetProjectBinPath(const project::ProjectSettings& settings);

enum MainTabbarSelection
{
	MAIN,
	ANIMATION,
};

static MainTabbarSelection tab = MAIN;

static void SaveINIToCorrectBuffer()
{
	std::string* buf = &ui::ini_main_buf;
	switch (tab)
	{
	case MAIN:
		buf = &ui::ini_main_buf;
		break;
	case ANIMATION:
		buf = &ui::ini_anim_buf;
		break;
	default:
		LOGERRORF("[{}] buttowski" __FUNCTION__);
		break;
	}

	*buf = ImGui::SaveIniSettingsToMemory();
}

// #TODO move somewhere else 
static void CheckScriptReload()
{
	if (!should_reload)
		return;

	bool game_was_playing = false;
	if (Engine::Get().GameStateIsPlaying())
	{
		game_was_playing = true;
		Engine::Get().StopGameSession();
	}

	Engine::Get().UpdateGameBinPaths(LIB_FILE_PREFIX + project::current_project.name + "_Game" + LIB_FILE_EXT, GetProjectBinPath(project::current_project).string());

	Engine::Get().LoadGameScripts();

	if (game_was_playing)
		Engine::Get().StartGameSession();

	should_reload = false;
}

void ui::engine_ui(ImGuiContext* ctx)
{
	ImGui::SetCurrentContext(ctx);

	constexpr auto dock_window_flags =
		ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiButtonFlags_NoNavFocus;

	ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos);

	static bool project_load_popup_select = false;

	static bool view_settings = false;
	static bool view_text_editor = false;
	static bool view_fsm_editor = true;

	static project::ProjectSettings settings{};

	static FileBrowser fBrowser(std::filesystem::current_path().string());
	static GameAssetsBrowser asset_browser(settings.project_path);
	static TextEditor text_edit;
	static AnimationEditor anim_editor;
	static FSMGraphEditor fsm_graph_editor;
	static MessageQueue message_queue{ {180, 100}, MessageQueuePlacement::RIGHT };

    // set default project settings
    static bool once = true;
    if (once)
    {
#ifdef _WIN32
        settings.project_gen_type = project::PROJECT_TYPE::VS_2022;
#else
        settings.project_gen_type = project::PROJECT_TYPE::Makefile;
#endif

		browser = &asset_browser;
        once = false;
    }

	CheckScriptReload();

	if (Scene::current_scene.removed_from_scene)
	{
		if (selected_obj)
			if (!Scene::current_scene.GetSceneObject(selected_obj->name))
			{
				selected_obj = nullptr;
			}
		selected_objects.clear(); // #TODO: lilazy
		Scene::current_scene.removed_from_scene = false;
	}

	//LOGDEBUGF("{} {}", ImGui::GetMousePos().x, ImGui::GetMousePos().y);

	ImGui::Begin("DockSpace", nullptr, dock_window_flags);
	ImGui::DockSpace(ImGui::GetID("DockSpace"));

	message_queue.Show();
	
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Engine"))
		{
			if (ImGui::MenuItem("Test child window"))
			{
				Engine::Get().AddViewport(sf::VideoMode(500, 500), "abc", sf::Style::Close | sf::Style::Resize);
			}	
			if (ImGui::MenuItem("Test button"))
			{
				MessageQueueMessage msg;
				msg.category = MessageCategory::ENGINE;
				msg.message = "Successfully initialized engine";
				msg.title = "Engine Init";
				msg.show_time_ms = 2000.f;
				msg.type = MessageType::INFO;
				message_queue.AddToMessageQueue(msg);
			}
			if (ImGui::MenuItem("Create Project.."))
			{
				settings.engine_path = GetEngineDirectory().string();
				LOGDEBUGF("Engine path: {}", settings.engine_path);
				ImGui::PushOverrideID(project_creation_popup_id);
				ImGui::OpenPopup("CreateProject");
				ImGui::PopID();

			}
			if (ImGui::MenuItem("Load Project.."))
			{
				project_load_popup_select = true;

				ImGui::PushOverrideID(project_load_popup_id);
				ImGui::OpenPopup("LoadProject");
				ImGui::PopID();
			}
			ImGui::BeginDisabled(project::current_project.name.empty());
			if (ImGui::MenuItem("Update Project"))
			{
				if (settings.engine_path.empty())
					settings.engine_path = GetEngineDirectory().string();

				if (!project::Update(settings, project::current_project.project_path))
					LOGERRORF("Failed to update project {}", project::current_project.project_path);
			}
			if (ImGui::MenuItem("Package.."))
			{
				ImGui::PushOverrideID(project_package_popup_id);
				ImGui::OpenPopup("PackageProject");
				ImGui::PopID();
			}
			if (ImGui::MenuItem("Project Settings.."))
			{
				ImGui::PushOverrideID(project_settings_popup_id);
				ImGui::OpenPopup("ProjectSettings");
				ImGui::PopID();
			}
			ImGui::EndDisabled();
			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu("Scripts"))
		{
			ImGui::BeginDisabled(project::current_project.name.empty());
			if (ImGui::MenuItem("Reload"))
				should_reload = true;

			ImGui::MenuItem("Auto Reload", nullptr, nullptr, &auto_reload);

			ImGui::EndDisabled();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Scene"))
		{
			Scene& scene = Scene::current_scene;

			if (scene.objects_all.empty())
			{
				ImGui::BeginDisabled();
			}
			if (ImGui::MenuItem("Save"))
			{
				if (!scene.objects_all.empty())
				{
					ImGui::PushOverrideID(save_scene_popup_id);
					ImGui::OpenPopup("SaveScene");
					ImGui::PopID();
				}
			}
			if (scene.objects_all.empty())
			{
				ImGui::EndDisabled();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Views"))
		{
			ImGui::MenuItem("Settings", nullptr, &view_settings);
			ImGui::MenuItem("Text Editor", nullptr, &view_text_editor);

			ImGui::EndMenu();
		}

		if (ImGui::BeginTabBar("MainTabbarSelection"))
		{
			ImGuiTabItemFlags flags = ImGuiTabItemFlags_None;

			if (tab == MAIN)
				flags |= ImGuiTabItemFlags_SetSelected;

			if (ImGui::BeginTabItem("Main", nullptr, flags))
			{
				if (tab != MAIN && ImGui::IsItemHovered() && ImGui::IsMouseDown(0))
				{
					SaveINIToCorrectBuffer();
					tab = MAIN;

					load_ini = true;
					ini_buf = ini_main_buf.c_str();
				}
				ImGui::EndTabItem();
			}

			flags = ImGuiTabItemFlags_None;

			if (tab == ANIMATION)
				flags |= ImGuiTabItemFlags_SetSelected;

			if (ImGui::BeginTabItem("Animation", nullptr, flags))
			{
				if (tab != ANIMATION && ImGui::IsItemHovered() && ImGui::IsMouseDown(0))
				{
					SaveINIToCorrectBuffer();
					tab = ANIMATION;

					load_ini = true;
					ini_buf = ini_anim_buf.c_str();
				}
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::EndMenuBar();
	}

	ImGui::End();

	/// menu bar popup dialogs 

	ImGui::PushOverrideID(project_creation_popup_id);
	if (ImGui::BeginPopupModal("CreateProject"))
	{		
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		static bool created_project = false;
		static project::CREATE_PROJECT_RES_INFO cpri;
		static std::vector<project::ProjectTemplate> project_templates{};
		static std::string selected_template;

		// when we want to refresh we clear it 
		if (project_templates.empty())
		{
			project_templates = project::GetAvailableTemplates(settings.engine_path);
			if (project_templates.empty())
			{
				LOGERROR("No templates found");
				project_templates.emplace_back();
			}
			else
				selected_template = project_templates[0].name;
		}

		if (created_project)
		{
			if (cpri.res != project::CREATE_PROJECT_RES::OK)
			{
				if (ImGui::Button("Retry"))
				{
					project_templates.clear();
					created_project = false;
				}

				ImGui::TextColored({ 1,0,0,1 }, "%s", format_str("Project creation failed: {}, {}", cpri.res, cpri.description).c_str());
			}
			else
			{
				if (ImGui::Button("Done"))
				{
					project_templates.clear();
					created_project = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::Text("Project created successfully");
			}
		}
		else
		{
			static char name[30];
			ImGui::TextDisabled("Engine path %s", settings.engine_path.c_str());
			if (ImGui::InputText("name", name, 30))
				settings.name = name;

            const char* project_types[] = {"Visual Studio 2022", "Visual Studio 2019", "Visual Studio 2015", "Makefile", "Codelite", "Xcode"};

#ifdef _WIN32
            static const char* current_type_preview = "Visual Studio 2022";
#else
            static const char* current_type_preview = "Makefile";
#endif

            if (ImGui::BeginCombo("project file type", current_type_preview))
            {
                for (int i = 0; i < IM_ARRAYSIZE(project_types); i++)
                {
                    bool is_selected = (int)settings.project_gen_type == i;
                    if (ImGui::Selectable(project_types[i], is_selected)) {
                        settings.project_gen_type = project::PROJECT_TYPE(i);
                        current_type_preview = project_types[i];
                    }
                }
                ImGui::EndCombo();
            }

			ImGui::Text("Selected Path %s", settings.project_path.empty() ? "?" : settings.project_path.string().c_str());

			if (ImGui::Button("Select Project Directory"))
			{
				auto selected = GetPathDialog("Select project directory", settings.engine_path);
				settings.project_path = selected;
			}

			if (ImGui::BeginChild("Project templates", { 0, ImGui::GetContentRegionAvail().y / 2 }, true))
			{
				if (ImGui::Button("Refresh"))
					project_templates = project::GetAvailableTemplates(settings.engine_path);

				static size_t selected = 0;
				if (ImGui::BeginTable("Project templates table", 4, ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_Borders))
				{
					// #TODO: add sorting 

					ImGui::TableSetupColumn("Preview", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed);
					ImGui::TableSetupColumn("Name");
					ImGui::TableSetupColumn("Description");
					ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_NoSort);
					ImGui::TableHeadersRow();

					for (size_t i = 0; i < project_templates.size(); i++)
					{
						const project::ProjectTemplate& pt = project_templates[i];
						
						ImGui::TableNextRow();
						ImGui::TableNextColumn();

						if (pt.image_preview_loaded)
						{
							ImGui::Image(pt.image_preview, { 50, 50 }, sf::Color::White);
							if (ImGui::IsItemHovered())
							{
								if (ImGui::BeginTooltip())
								{
									static float scaling = 150;
									scaling += ImGui::GetIO().MouseWheel * 10;
									ImGui::Image(pt.image_preview, { scaling, scaling }, sf::Color::White);
									ImGui::EndTooltip();
								}
							}
						}
						ImGui::TableNextColumn();

						if (ImGui::Selectable(pt.name.c_str(), selected == i, ImGuiSelectableFlags_SpanAllColumns, {0, 50.f}))
						{
							selected = i;
							selected_template = pt.name;
						}
						
						ImGui::TableNextColumn();
						if (pt.description.empty())
							ImGui::TextColored({1, 1, 1, 0.7f}, "No decription provided");
						else
							ImGui::Text("%s", pt.description.c_str());
						ImGui::TableNextColumn();
						ImGui::Text("%s", pt.path.string().c_str());
					}

					ImGui::EndTable();
				}
			}
			ImGui::EndChild();

			ImGui::Text("Selected Template: %s", selected_template.c_str());
			if (!settings.name.empty() && !settings.project_path.empty() && !settings.engine_path.empty() && !selected_template.empty())
			{
				if (ImGui::Button("Create"))
				{
					created_project = true;

					cpri = Create(settings, selected_template);

					if (cpri.res != project::CREATE_PROJECT_RES::OK)
					{
						fBrowser.SetPath(settings.project_path);
						LOGERRORF("{} {}", cpri.res, cpri.description);
					}
					else
					{
						LOGDEBUGF("{} {}", cpri.res, cpri.description);
						asset_browser.SetAssetPath((std::filesystem::path(settings.project_path) / (settings.name + "_GAME") / "src" / "assets").string());
					}

					if (!std::filesystem::exists(project::current_project.engine_path))
						project::current_project.engine_path = GetEngineDirectory().string();
						
                    Engine::Get().UpdateGameBinPaths(LIB_FILE_PREFIX + settings.name + LIB_FILE_EXT, GetProjectBinPath(settings).string());
				}

			}
		}

		ImGui::EndPopup();
	}
	ImGui::PopID();

	ImGui::PushOverrideID(project_load_popup_id);
	if (ImGui::BeginPopupModal("LoadProject"))
	{
		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();

		static project::LOAD_PROJECT_RES_INFO lri;
		static std::string path;

		if (project_load_popup_select)
		{
			// TODO: cache toad projects folder s
			path = GetPathFile(std::filesystem::current_path().string(), "Toad Project (*.TOADPROJECT)\0*.TOADPROJECT\0");

			if (!path.empty()){
				lri = project::Load(path);

				std::string game_folder;

				for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::path(path).parent_path()))
				{
					if (entry.is_directory() &&
						entry.path().filename().string().find("Game") != std::string::npos)
					{
						game_folder = entry.path().filename().string();
						break;
					}
				}

				if (lri.res == project::LOAD_PROJECT_RES::OK)
				{
					Engine::Get().UpdateGameBinPaths(LIB_FILE_PREFIX + project::current_project.name + "_Game" + LIB_FILE_EXT, GetProjectBinPath(project::current_project).string());

					Engine::Get().LoadGameScripts();

					if (!std::filesystem::exists(project::current_project.engine_path))
						project::current_project.engine_path = GetEngineDirectory().string();

					asset_browser.SetAssetPath((std::filesystem::path(path).parent_path() / game_folder / "src" / "assets").string());
					fBrowser.SetPath((std::filesystem::path(path).parent_path() / game_folder).string());

					ImGui::CloseCurrentPopup();
				}
			}
			else
			{
				lri.description = "Selected file is empty";
			}

			project_load_popup_select = false;
		}
		else
		{
			ImGui::TextColored({ 1,0,0,1 }, "Failed to load project, %s", lri.description.empty() ? "?" : lri.description.c_str());
			ImGui::Text("path %s", path.c_str());
			if (ImGui::Button("Select Project"))
			{
				project_load_popup_select = true;
			}
		}

		ImGui::EndPopup();
	}
	ImGui::PopID();

	ImGui::PushOverrideID(project_package_popup_id);
	if (ImGui::BeginPopupModal("PackageProject"))
	{
		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
		}
		static std::string output_path;
		static bool debug_build = false;

		ImGui::Text("selected path: %s", output_path.c_str());
		if (ImGui::Button("Select output directory"))
		{			
			output_path = GetPathDialog("select output directory", std::filesystem::current_path().string());
		} 
		ImGui::Checkbox("Debug build", &debug_build);
		ImGui::BeginDisabled(output_path.empty());
		if (ImGui::Button("Create"))
		{
			static Package package;
			std::filesystem::path proj_file;

			std::filesystem::path project_directory = project::current_project.project_path;
			if (!std::filesystem::is_directory(project_directory))
				project_directory = project_directory.parent_path();

			for (const auto& entry : std::filesystem::directory_iterator(project_directory))
			{
				if (entry.path().has_extension() && entry.path().extension() == FILE_EXT_TOADPROJECT)
				{
					proj_file = entry.path();
					break;
				}
			}

			if (misc::current_editor.path.empty())
			{
				// TODO :
				auto editors = misc::FindEditors();
				if (editors.empty())
				{
					LOGERROR("No editors found .... ");
				}
				else
				{
					misc::current_editor = editors[0];
				}
			}

			if (settings.engine_path.empty())
			{
				settings.engine_path = GetEngineDirectory().string();
			}
			Package::CreatePackageParams p;
			p.project_file_path = proj_file;
			p.output_dir_path = output_path;
			p.build_system_file_path = misc::current_editor.path;
			p.engine_path = settings.engine_path;
			p.is_debug = debug_build;
			package.CreatePackage(p);
		}
		ImGui::EndDisabled();

		ImGui::EndPopup();
	}
	ImGui::PopID();

	ImGui::PushOverrideID(project_package_popup_id);
	if (ImGui::BeginPopupModal("ProjectSettings"))
	{
		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();

		// #TODO: implement these

		// application name 
		// application fps 
		// application resolution
		// application window flags

		// Time fixed time step

		// Package options 

		// etc 

		ImGui::EndPopup();
	}
	ImGui::PopID();

	ImGui::PushOverrideID(save_scene_popup_id);
	if (ImGui::BeginPopupModal("SaveScene"))
	{
		static char scene_name[50];
		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::InputText("name", scene_name, sizeof(scene_name));

		if (ImGui::Button("Create"))
		{
			Scene::current_scene.name = scene_name;
			SaveScene(Scene::current_scene, asset_browser.GetAssetPath());
		}

		ImGui::EndPopup();
	}
	ImGui::PopID();

	if (view_settings)
	{
		ImGui::Begin("Settings", &view_settings);
		{
			ImGui::Text("FPS %.1f", 1.f / Time::GetDeltaTime());

			if (ImGui::TreeNode("game scenes order"))
			{
				static std::vector<std::filesystem::path> scenes;

				HelpMarker("only searches for scene files in asset folder");
				static bool refresh = true;
				if (ImGui::Button("refresh"))
				{
					refresh = true;
					scenes.clear();
				}


				const auto& asset_path = asset_browser.GetAssetPath();
				if (!fs::exists(asset_path))
				{
					ImGui::TextColored({ 1,0,0,1 }, "Can't find asset folder in %s", asset_path.string().c_str());
				}
				else
				{
					if (refresh)
					{
						for (const auto& entry : std::filesystem::recursive_directory_iterator(asset_path))
						{
							if (entry.path().has_extension() && entry.path().extension() == ".TSCENE")
								scenes.push_back(entry);
						}

						refresh = false;
					}
				}

				ImGui::Indent();
				int move_to = -1, move_from = -1;

				for (int i = 0; i < scenes.size(); i++)
				{
					ImGui::Selectable(format_str("[{}] {}", i, scenes[i].filename().string()).c_str());

					ImGuiDragDropFlags src_flags = 0;
					src_flags |= ImGuiDragDropFlags_SourceNoDisableHover;
					src_flags |= ImGuiDragDropFlags_SourceNoHoldToOpenOthers;
					src_flags |= ImGuiDragDropFlags_SourceNoPreviewTooltip;
					if (ImGui::BeginDragDropSource(src_flags))
					{
						ImGui::SetDragDropPayload("dnd scene order", &i, sizeof(int));
						ImGui::EndDragDropSource();
					}

					if (ImGui::BeginDragDropTarget())
					{
						ImGuiDragDropFlags target_flags = 0;
						target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
						target_flags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect; // Don't display the yellow rectangle
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dnd scene order", target_flags))
						{
							move_from = *(const int*)payload->Data;
							move_to = i;
							std::swap(scenes[move_from], scenes[move_to]);
							ImGui::SetDragDropPayload("dnd scene order", &move_to, sizeof(int));
						}
						ImGui::EndDragDropTarget();
					}
				}

				ImGui::Unindent();
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("all attached scripts"))
			{
				for (const auto& obj : Scene::current_scene.objects_all)
				{
					ImGui::Text("name %s %p", obj->name.c_str(), obj.get());
					for (const auto& [namescript, script] : obj->GetAttachedScripts())
					{
						ImGui::Text("script %s %p", namescript.c_str(), script);
					}
				}

				ImGui::TreePop();
			}
			ImGui::End();
		}
	}

	if (tab == MAIN)
	{
		// SCENE/HIERARCHY
		ui::ShowSceneHierarchy();

		ImGui::Begin("Inspector", nullptr);
		{
			if (hierarchy_clicked_object)
				inspector_ui = std::bind(&ui::object_inspector, std::ref(selected_obj), std::ref(asset_browser));

			if (inspector_ui)
				inspector_ui();
		
			ImGui::End();
		}

		ImGui::Begin("Game view", nullptr, ImGuiWindowFlags_HorizontalScrollbar);
		{	
			// button from viewport 
			static fs::path last_scene_path;

			if (!Engine::Get().GameStateIsPlaying())
			{
				if (ImGui::Button("Play"))
				{
					if (reload_scene_on_stop)
						last_scene_path = Scene::current_scene.path;

					Engine::Get().StartGameSession();
				}
			}
			else
			{
				if (ImGui::Button("Stop"))
				{
					Engine::Get().StopGameSession();

					if (reload_scene_on_stop)
						Scene::current_scene = LoadScene(last_scene_path, asset_browser.GetAssetPath());
				}
			}

			auto content_size = ImGui::GetContentRegionAvail();
			// resize but keep aspect ratio
			constexpr float ar = 16.f / 9.f;
			float image_width = content_size.x;
			float image_height = content_size.x / ar;
		
			Camera* cam = Camera::GetActiveCamera();

			if (cam == nullptr)
			{
				const char* err_msg = "No camera's in scene, please create a camera";
				ImVec2 size = ImGui::CalcTextSize(err_msg) / 2;
				ImVec2 image_cursor_pos = ImGui::GetCursorPos();
				ImGui::SetCursorPos({
					image_cursor_pos.x + image_width / 2 - size.x,
					image_cursor_pos.y + size.y + 10
					});

				ImGui::TextColored({ 1, 0, 0, 1 }, "%s", err_msg);
			}
			else
			{
				auto& window_texture = Engine::Get().GetWindowTexture();

				auto content_size = ImGui::GetContentRegionAvail();

				if (image_height > content_size.y)
				{
					image_height = content_size.y;
					image_width = content_size.y * ar;
				}

				// we want texture in the center 
				float pady = 25; // #TODO: find the actual imgui style property 
				ImGui::SetCursorPos({
					(content_size.x - image_width) * 0.5f,
					(content_size.y - image_height + pady) * 0.5f
					});

				const ImVec2 pos = ImGui::GetCursorScreenPos();

				ImGui::Image(window_texture, { image_width, image_height }, sf::Color::White);
				if (ImGui::IsItemClicked())
				{
					Engine::Get().capture_mouse = true;

					if (!Engine::Get().mouse_visible_prev)
						Mouse::SetVisible(false);
				}
				if (ImGui::IsKeyPressed(ImGuiKey_Escape))
				{
					Mouse::SetVisible(true);

					Engine::Get().capture_mouse = false;
					Engine::Get().mouse_visible_prev = Engine::Get().mouse_visible;
					Engine::Get().mouse_visible = true;
				}
				if (ImGui::IsWindowHovered())
				{
					Engine::Get().viewport_size = { (int)image_width, (int)image_height };

					if (cam)
					{
						Engine::Get().interacting_camera = cam;
						Engine::Get().interacting_texture = &window_texture;
						float f_x = cam->GetSize().x / image_width;
						float f_y = cam->GetSize().y / image_height;

						Engine::Get().relative_mouse_pos = {
						(int)((ImGui::GetMousePos().x - pos.x) * f_x),
						(int)((ImGui::GetMousePos().y - pos.y) * f_y) };
					}
					else
						Engine::Get().interacting_camera = &Engine::Get().GetEditorCamera();
				}
			}

			ImGui::End();
		}
		
		ui::ShowViewport(asset_browser);

		if (ImGui::Begin("File Browser"))
		{
			fBrowser.Show();

			if (fBrowser.IsDoubleClicked())
			{
				auto file = std::filesystem::path(fBrowser.GetSelectedFile());
				if (file.has_extension())
				{
					auto ext = file.extension().string();

					if (ext == ".TSCENE")
					{
						if (asset_browser.GetAssetPath().empty())
						{
							Scene::SetScene(&LoadScene(file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
							scene_history.asset_folder = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
							scene_history.scene = &Scene::current_scene;
							if (scene_history.SaveState())
								saved_scene = false;
						}
						else
						{
							Scene::SetScene(&LoadScene(file, asset_browser.GetAssetPath()));
							scene_history.asset_folder = asset_browser.GetAssetPath();
							scene_history.scene = &Scene::current_scene;
							if (scene_history.SaveState())
								saved_scene = false;
						}
						is_scene_loaded = true;
						selected_obj = nullptr;
					}
				}
			}
		}
		ImGui::End();
	}
	else if (tab == ANIMATION)
	{
		anim_editor.Show(nullptr);

		ImGui::Begin("Inspector", nullptr);
		{
			if (hierarchy_clicked_object)
				inspector_ui = std::bind(ui::object_inspector, std::ref(selected_obj), std::ref(asset_browser));

			if (inspector_ui)
				inspector_ui();

			ImGui::End();
		}
	}

	asset_browser.Show();
	if (asset_browser.loaded_scene)
	{
		selected_obj = nullptr;
		scene_history.asset_folder = asset_browser.GetAssetPath();
		scene_history.scene = &Scene::current_scene;
		scene_history.SaveState();
		is_scene_loaded = true;
	}

	if (view_text_editor)
	{
		ImGui::Begin("Text Editor", &view_text_editor);

		if (!fBrowser.GetSelectedFile().ends_with(PATH_SEPARATOR))
		{
			text_edit.Show(fBrowser.GetSelectedFileContent());
		}
		else
		{
			ImGui::Text("Select a file");
		}

		ImGui::End();
	}

	if (view_fsm_editor)
		fsm_graph_editor.Show(&view_fsm_editor, asset_browser);

	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_ModSuper))
	{
		if (ImGui::IsKeyPressed(ImGuiKey_Z))
		{
			if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
			{
				if (Engine::Get().GameStateIsPlaying())
					Engine::Get().StopGameSession();

				selected_obj = nullptr;
				selected_objects.clear();
				scene_history.Redo();
			}
			else
			{
				if (Engine::Get().GameStateIsPlaying())
					Engine::Get().StopGameSession();

				selected_obj = nullptr;
				selected_objects.clear();
				scene_history.Undo();
			}
		}
	}
}

void ui::update_ini()
{
	if (load_ini)
	{
		if (load_ini_file)
		{
			// will start on tab main

			if (fs::exists("imgui-main.ini"))
				ImGui::LoadIniSettingsFromDisk("imgui-main.ini");
			else if (fs::exists("imgui.ini"))
				ImGui::LoadIniSettingsFromDisk("imgui.ini");

			// check others 
			if (fs::exists("imgui-anim.ini"))
				ini_anim_buf = GetFileContents("imgui-anim.ini");

			load_ini_file = false;
		}
		else
		{
			Engine::Get().RecreateImGuiContext();
			ImGui::SetCurrentContext(Engine::Get().GetImGuiContext());
			if (ini_buf)
				ImGui::LoadIniSettingsFromMemory(ini_buf);
		}

		load_ini = false;
	}
}

void ui::event_callback(const sf::Event& e)
{
	switch (e.type)
	{
	case sf::Event::KeyReleased:
	case sf::Event::MouseButtonReleased:
		if (is_scene_loaded)
			scene_history.SaveState();
		break;
	case sf::Event::GainedFocus:
		if (browser)
			browser->refresh = true;
		if (auto_reload)
			should_reload = true;
		break;
	default: 
		break;
	}
}

void ui::editor_texture_draw_callback(sf::RenderTexture& texture)
{
	const Camera* active_cam = Camera::GetActiveCamera();
	if (!show_grid || !active_cam)
	{
		return;
	}
	const Camera& cam = Engine::Get().GetEditorCamera();
	static auto rect = sf::RectangleShape();
	rect.setFillColor(sf::Color::Transparent);
	rect.setOutlineThickness(2.f);
	rect.setOutlineColor(sf::Color(255, 255, 255, 80));
	rect.setSize(Vec2f{ (float)grid_size.x, (float)grid_size.y });

	for (float i = active_cam->GetPosition().x - grid_size.x * (active_cam->GetSize().x / grid_size.x); i < grid_size.x * (active_cam->GetSize().x / grid_size.x); i += (float)grid_size.x)
	{
		for (float j = active_cam->GetPosition().y - grid_size.y * (active_cam->GetSize().y / grid_size.y); j < grid_size.y * (active_cam->GetSize().y / grid_size.y); j += (float)grid_size.y)
		{
			rect.setPosition(Vec2f{ i, j });
			texture.draw(rect);
		}
	}
	
}

void ui::save_ini_files(int exit_code)
{
	SaveINIToCorrectBuffer();
	LOGDEBUGF("[{}] Saving ini data main: {}", __FUNCTION__, ini_main_buf.size());
	std::ofstream main_ini("imgui-main.ini", std::ofstream::out | std::ofstream::trunc);
	if (main_ini.is_open())
	{
		main_ini << ini_main_buf;
		main_ini.close();
	}

	LOGDEBUGF("[{}] Saving ini data anim: {}", __FUNCTION__, ini_anim_buf.size());
	std::ofstream anim_ini("imgui-anim.ini", std::ofstream::out | std::ofstream::trunc);
	if (anim_ini.is_open())
	{
		anim_ini << ini_anim_buf;
		anim_ini.close();
	}

}

void ui::HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(450.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

std::filesystem::path GetEngineDirectory()
{
    std::filesystem::path res;
	res = misc::GetExePath();

#ifdef TOAD_DISTRO
	return res.parent_path();
#else
	if (!std::filesystem::exists(res) || !std::filesystem::is_directory(res))
	{
		std::filesystem::path parent;
		do {
			parent = res.parent_path();
			for (const auto& entry : std::filesystem::directory_iterator(parent))
			{
				if (std::filesystem::is_directory(entry.path()))
					continue;

				if (entry.path().filename().string().find("Onion.sln") != std::string::npos || 
					entry.path().filename().string().find("Makefile") != std::string::npos)
				{
					return parent;
				}
			}
			res = parent;
		} while (parent.has_parent_path()); // #TODO always returns true 
	}
#endif

    return res;
}

std::filesystem::path GetProjectBinPath(const project::ProjectSettings& settings)
{
	std::filesystem::path p = settings.project_path;
	if (!std::filesystem::is_directory(p)) 
		p = p.parent_path();

	if (!std::filesystem::exists(p))
		return "";

    for (const auto& entry : std::filesystem::directory_iterator(p))
    {
        if (entry.path().filename().string().find("bin") != std::string::npos)
        {
            for (const auto& entry2 : std::filesystem::directory_iterator(entry.path()))
            {
                if (entry2.path().filename().string().find(PROJECT_BIN_SEARCH_FOR) != std::string::npos)
                {
                    return entry2.path();
                }
            }
        }
    }

    LOGWARNF("Can't find binary directory in {}, looking for a binary compiled with the '{}' configuration", settings.project_path, PROJECT_BIN_SEARCH_FOR);
    return "";
}

bool ImGui::SliderVec2(std::string_view label, float* x, float* y, float min, float max)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const float w = ImGui::CalcItemWidth();
	bool res = false;

	const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, style.FramePadding.y * 2.0f));

	std::string label_final = "##x";
	label_final += label;

	ImGui::Text("%s", label.data());
	ImGui::SameLine();
	ImGui::PushItemWidth(frame_bb.GetWidth() / 2.f - style.FramePadding.x);
	if (ImGui::DragFloat(label_final.c_str(), x, 1.0f, min, max))
		res = true;
	ImGui::SameLine();
	label_final = "##y";
	label_final += label;
	if (ImGui::DragFloat(label_final.c_str(), y, 1.0f, min, max))
		res = true;
	ImGui::PopItemWidth();

	return res;
}

bool ImGui::SliderVec2i(std::string_view label, int* x, int* y, int min, int max)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const float w = ImGui::CalcItemWidth();
	bool res = false;

	const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, style.FramePadding.y * 2.0f));

	std::string label_final = "##x";
	label_final += label;

	ImGui::Text("%s", label.data());
	ImGui::SameLine();
	ImGui::PushItemWidth(frame_bb.GetWidth() / 2.f - style.FramePadding.x);
	if (ImGui::DragInt(label_final.c_str(), x, 1.0f, min, max))
		res = true;
	ImGui::SameLine();
	label_final = "##y";
	label_final += label;
	if (ImGui::DragInt(label_final.c_str(), y, 1.0f, min, max))
		res = true;
	ImGui::PopItemWidth();

	return res;
}

bool ImGui::SliderVec2i(std::string_view label, Vec2i* v, int min, int max)
{
	return ImGui::SliderVec2i(label, &v->x, &v->y, min, max);
}

bool ImGui::SliderVec2(std::string_view label, Vec2f* v, float min, float max)
{
	return ImGui::SliderVec2(label, &v->x, &v->y, min, max);
}

#endif 
