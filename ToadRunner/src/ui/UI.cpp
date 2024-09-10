#include "pch.h"

#ifdef TOAD_EDITOR
#include <algorithm>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "engine/Engine.h"

#include "UI.h"

#include "imgui-SFML.h"
#include "FileBrowser.h"
#include "AnimationEditor.h"
#include "GameAssetsBrowser.h"
#include "FSMGraphEditor.h"
#include "TextEditor.h"
#include "MessageQueue.h"

#include "engine/systems/build/Package.h"
#include "project/ToadProject.h"
#include "SceneHistory.h"
#include "utils/FileDialog.h"
#include "engine/utils/Helpers.h"
#include "engine/systems/Animation.h"
#include "engine/Types.h"
 
using json = nlohmann::ordered_json;

constexpr int i8_min = std::numeric_limits<int8_t>::min();
constexpr int i8_max = std::numeric_limits<int8_t>::max();
constexpr int i16_min = std::numeric_limits<int16_t>::min();
constexpr int i16_max = std::numeric_limits<int16_t>::max();
constexpr int i32_max = std::numeric_limits<int32_t>::max();

using namespace Toad;

static Toad::SceneHistory scene_history{};
static Toad::GameAssetsBrowser* browser = nullptr; // #TODO: TEMP?
static bool is_scene_loaded = false;

// get installed directory (distro)
// get engine solution directory (source)
extern std::filesystem::path GetEngineDirectory();
extern std::filesystem::path GetProjectBinPath(const project::ProjectSettings& settings);

void ui::engine_ui(ImGuiContext* ctx)
{
	ImGui::SetCurrentContext(ctx);

	constexpr auto dock_window_flags =
		ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiButtonFlags_NoNavFocus;

	ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos);

	static ImGuiID project_creation_popup_id = ImHashStr("CreateProject");
	static ImGuiID project_load_popup_id = ImHashStr("LoadProject");
	static ImGuiID project_package_popup_id = ImHashStr("PackageProject");
	static ImGuiID project_settings_popup_id = ImHashStr("ProjectSettings");
	static ImGuiID save_scene_popup_id = ImHashStr("SaveScene");
	static ImGuiID scene_modify_popup_id = ImHashStr("SceneModifyPopup");
	static bool project_load_popup_select = false;

	static project::ProjectSettings settings{};
	static Toad::FileBrowser fBrowser(std::filesystem::current_path().string());
	static Toad::GameAssetsBrowser asset_browser(settings.project_path);
	static Toad::TextEditor textEditor;
	static Toad::AnimationEditor anim_editor;
	static Toad::FSMGraphEditor fsm_graph_editor;
	static Toad::MessageQueue message_queue{{180, 100}, Toad::MessageQueuePlacement::RIGHT};

	static bool view_settings = false;
	static bool view_text_editor = false;
	static bool view_fsm_editor = true;

	// when to show '*' for unsaved changes
	static bool saved_scene = true;

	static Toad::Object* selected_obj = nullptr;
	static std::set<std::string> selected_objects = {};

	bool hierarchy_clicked_object = false;
	
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
				Toad::Engine::Get().AddViewport(sf::VideoMode(500, 500), "abc", sf::Style::Close | sf::Style::Resize);
			}	
			if (ImGui::MenuItem("Test button"))
			{
				Toad::MessageQueueMessage msg;
				msg.category = Toad::MessageCategory::ENGINE;
				msg.message = "Successfully initialized engine";
				msg.title = "Engine Init";
				msg.show_time_ms = 2000.f;
				msg.type = Toad::MessageType::INFO;
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
			{
				if (Toad::Engine::Get().GameStateIsPlaying())
				{
					Toad::Engine::Get().StopGameSession();
				}

				Toad::Engine::Get().UpdateGameBinPaths(LIB_FILE_PREFIX + project::current_project.name + "_Game" + LIB_FILE_EXT, GetProjectBinPath(project::current_project).string());

				Toad::Engine::Get().LoadGameScripts();
			}
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

				ImGui::TextColored({ 1,0,0,1 }, "%s", Toad::format_str("Project creation failed: {}, {}", cpri.res, cpri.description).c_str());
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
				auto selected = Toad::GetPathDialog("Select project directory", settings.engine_path);
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
							ImGui::Text(pt.description.c_str());
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
						
                    Toad::Engine::Get().UpdateGameBinPaths(LIB_FILE_PREFIX + settings.name + LIB_FILE_EXT, GetProjectBinPath(settings).string());
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
			path = Toad::GetPathFile(std::filesystem::current_path().string(), "Toad Project (*.TOADPROJECT)\0*.TOADPROJECT\0");

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
					Toad::Engine::Get().UpdateGameBinPaths(LIB_FILE_PREFIX + project::current_project.name + "_Game" + LIB_FILE_EXT, GetProjectBinPath(project::current_project).string());

					Toad::Engine::Get().LoadGameScripts();

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
			output_path = Toad::GetPathDialog("select output directory", std::filesystem::current_path().string());
		} 
		ImGui::Checkbox("Debug build", &debug_build);
		ImGui::BeginDisabled(output_path.empty());
		if (ImGui::Button("Create"))
		{
			static Toad::Package package;
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
			Toad::Package::CreatePackageParams p;
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
				if (!exists(asset_path))
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
					ImGui::Selectable(Toad::format_str("[{}] {}", i, scenes[i].filename().string()).c_str());

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

	// SCENE/HIERARCHY
	ImGui::Begin("Scene", nullptr);
	{
		std::vector<std::pair<std::string, std::string>> set_object_childs = {};
		int index = 0;
		static size_t prev_cursor_index = 0;
		static size_t cursor_index = 0;
		static bool cursor_index_is_under = false;
		static bool check_range = false;
		static std::string drag_drop_extra_check_parent;
		static std::string drag_drop_extra_check_child;

		std::vector<std::string> scene_objects{};
		std::set<std::string> scene_objects_set{};

		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_ModSuper))
		{
			Toad::Scene& scene = Scene::current_scene;

			if (ImGui::IsKeyPressed(ImGuiKey_S))
			{
				if (!scene.path.empty())
				{
					saved_scene = true;
					SaveScene(scene, scene.path.parent_path());
				}
			}
			if (ImGui::IsKeyPressed(ImGuiKey_C))
			{
				std::vector<Toad::Object*> objects;
				size_t count = 0;
				if (selected_obj != nullptr)
				{
					count++;
					objects.emplace_back(selected_obj);
				}
				count += selected_objects.size();

				objects.reserve(count);

				for (const std::string& name : selected_objects)
				{
					objects.emplace_back(scene.GetSceneObject(name).get());
				}

				json data = scene.Serialize(objects);
				std::stringstream ss;
				ss << data;

				ImGui::SetClipboardText(ss.str().c_str());
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_V))
			{
				const char* data = ImGui::GetClipboardText();

				try
				{
					json data_json = json::parse(data);
					Toad::LoadSceneObjects(data_json, scene, "asset_browser.GetAssetPath()", false);
				}
				catch (const json::parse_error& e)
				{
					LOGERRORF("json parse error {}, check your clipboard", e.what());
				}
			}
		}

		ImGui::SeparatorText((saved_scene ? Scene::current_scene.name : Scene::current_scene.name + '*').c_str());
		bool ignore_mouse_click = false;
		std::queue<std::string> remove_objects_queue;

		// #TODO: fix drag and drop ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#&(*^!@(*&$^!(*&@$^*!(@
		const std::function<void(Toad::Object*, bool) > recursive_iterate_children = [&](Toad::Object* obj, bool dontchangeskip)
			{
				index++;
				static bool skip = false;

				const auto drag_drop = [](bool& skip, Toad::Object* obj) {
					if (ImGui::BeginDragDropTarget())
					{
						if (selected_obj && obj)
							LOGDEBUGF("dragging {1} to {0}", obj->name, selected_obj->name);
	
						drag_drop_extra_check_parent = obj->name;
						drag_drop_extra_check_child = selected_obj->name;
						
						if (ImGui::AcceptDragDropPayload("move object"))
						{
							if (selected_obj != nullptr)
							{
								selected_obj->SetParent(obj);
							}
							for (const std::string& o : selected_objects)
							{
								Toad::Object* as_object = Scene::current_scene.GetSceneObject(o).get();

								if (as_object != nullptr)
								{
									as_object->SetParent(obj);
								}
							}
						}
						ImGui::EndDragDropTarget();
					}
					if (ImGui::BeginDragDropSource())
					{
						if (!skip)
						{
							if (selected_obj != nullptr)
							{
								if (!selected_objects.contains(obj->name) && selected_obj->name != obj->name)
								{
									selected_objects.clear();
									selected_obj = obj;
								}
								ImGui::SetDragDropPayload("move object", obj->name.c_str(), obj->name.length());
							}
							else
							{
								selected_obj = obj;
							}
						}
						skip = true;
						ImGui::EndDragDropSource();
					}
				};

				if (!scene_objects_set.contains(obj->name))
				{
					scene_objects.emplace_back(obj->name);
					scene_objects_set.emplace(obj->name);
				}

				if (obj->GetChildren().empty())
				{
					if (ImGui::Selectable(obj->name.c_str(), selected_objects.contains(obj->name) || (selected_obj != nullptr && selected_obj->name == obj->name)))
					{
						if ((ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_ModSuper)) && selected_obj != nullptr && selected_obj->name != obj->name)
						{
							if (selected_objects.contains(obj->name))
							{
								selected_objects.erase(obj->name);
							}
							else
							{
								selected_objects.insert(obj->name);
							}
						}
						else if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && selected_obj != nullptr && selected_obj->name != obj->name)
						{
							check_range = true;
							cursor_index = index;
							cursor_index_is_under = cursor_index > prev_cursor_index;
						}
						else
						{
							prev_cursor_index = index;
							selected_obj = obj;
							selected_objects.clear();
						}
					}
					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
					{
						if (selected_objects.empty())
						{
							selected_obj = obj;
						}
						prev_cursor_index = index;

						ImGui::PushOverrideID(scene_modify_popup_id);
						ImGui::OpenPopup("SceneModifyPopup");
						ImGui::PopID();

						ignore_mouse_click = true;
					}
				}
				else
				{
					ImGuiTreeNodeFlags node_flags = 0;
					node_flags |= ImGuiTreeNodeFlags_OpenOnArrow;
					if (selected_objects.contains(obj->name) || (selected_obj != nullptr && selected_obj->name == obj->name))
					{
						node_flags |= ImGuiTreeNodeFlags_Selected;
					}

					if (ImGui::TreeNodeEx(obj->name.c_str(), node_flags))
					{
						// root/parent widget
						if (ImGui::IsItemHovered())
						{
							if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
							{
								if (selected_objects.empty())
								{
									selected_obj = obj;
								}
								prev_cursor_index = index;
								if (!ImGui::IsPopupOpen("SceneModifyPopup"))
								{
									ImGui::PushOverrideID(scene_modify_popup_id);
									ImGui::OpenPopup("SceneModifyPopup");
									ImGui::PopID();
									ignore_mouse_click = true;
								}
							}
							else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
							{
								if ((ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_ModSuper)) && selected_obj != nullptr && selected_obj->name != obj->name)
								{
									if (selected_objects.contains(obj->name))
									{
										selected_objects.erase(obj->name);
									}
									else
									{
										selected_objects.insert(obj->name);
									}
								}
								else if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && selected_obj != nullptr && selected_obj->name != obj->name)
								{
									check_range = true;
									cursor_index = index;
									cursor_index_is_under = cursor_index > prev_cursor_index;
								}
								else
								{
									prev_cursor_index = index;
									selected_obj = obj;
									selected_objects.clear();
								}
							}
						}

						if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
							drag_drop(skip, obj);

						for (Toad::Object* child : obj->GetChildrenAsObjects())
						{
							if (!scene_objects_set.contains(child->name))
							{
								scene_objects.emplace_back(child->name);
								scene_objects_set.emplace(child->name);
							}

							recursive_iterate_children(child, true);
							
							if (ImGui::IsItemHovered())
							{
								if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
								{
									if (selected_objects.empty())
									{
										selected_obj = child;
									}
									prev_cursor_index = index;
									if (!ImGui::IsPopupOpen("SceneModifyPopup"))
									{
										ImGui::PushOverrideID(scene_modify_popup_id);
										ImGui::OpenPopup("SceneModifyPopup");
										ImGui::PopID();
										ignore_mouse_click = true;
									}
								}
								else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
								{
									if ((ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_ModSuper)) && selected_obj != nullptr && selected_obj->name != child->name)
									{
										if (selected_objects.contains(child->name))
										{
											selected_objects.erase(child->name);
										}
										else
										{
											selected_objects.insert(child->name);
										}
									}
									else if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && selected_obj != nullptr && selected_obj->name != child->name)
									{
										check_range = true;
										cursor_index = index;
										cursor_index_is_under = cursor_index > prev_cursor_index;
									}
									else
									{
										prev_cursor_index = index;
										selected_obj = child;
										skip = true;
										selected_objects.clear();
									}
								}
							}

							if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
								drag_drop(skip, child);
						}

						ImGui::TreePop();
					}
					else
					{
						if (ImGui::IsItemHovered())
						{
							if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
							{
								if (selected_objects.empty())
								{
									selected_obj = obj;
								}
								prev_cursor_index = index;
								if (!ImGui::IsPopupOpen("SceneModifyPopup"))
								{
									ImGui::PushOverrideID(scene_modify_popup_id);
									ImGui::OpenPopup("SceneModifyPopup");
									ImGui::PopID();
									ignore_mouse_click = true;
								}
							}
							else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
							{
								if ((ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_ModSuper))  && selected_obj != nullptr && selected_obj->name != obj->name)
								{
									if (selected_objects.contains(obj->name))
									{
										selected_objects.erase(obj->name);
									}
									else
									{
										selected_objects.insert(obj->name);
									}
								}
								else if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && selected_obj != nullptr && selected_obj->name != obj->name)
								{
									check_range = true;
									cursor_index = index;
									cursor_index_is_under = cursor_index > prev_cursor_index;
								}
								else
								{
									prev_cursor_index = index;
									selected_obj = obj;
									selected_objects.clear();
								}
							}
						}
					}
				}

				if (!skip)
				{
					drag_drop(skip, obj);
				}

				if (!dontchangeskip)
					skip = false;
			};

		if (ImGui::IsKeyPressed(ImGuiKey_Delete))
		{
			if (selected_obj != nullptr)
			{
				selected_obj->Destroy();
				selected_obj = nullptr;
			}
			for (const std::string& s : selected_objects)
			{
				Scene::current_scene.GetSceneObject(s)->Destroy();
				remove_objects_queue.emplace(s);
			}
		}

		for (auto& obj : Scene::current_scene.objects_all)
		{
			// make sure we only iterate over root objects
			if (!obj->GetParent().empty())
			{
				continue;
			}

			recursive_iterate_children(obj.get(), false);
		}

		// do some extra checks beucase the recursive_iterate_children is not good 
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			if (!drag_drop_extra_check_parent.empty() || !drag_drop_extra_check_child.empty())
			{
				// get them as objects 
				auto child_obj = Scene::current_scene.GetSceneObject(drag_drop_extra_check_child);
				auto parent_obj = Scene::current_scene.GetSceneObject(drag_drop_extra_check_parent);
				if (!child_obj || !parent_obj)
				{
					drag_drop_extra_check_parent.clear();
					drag_drop_extra_check_child.clear();
				}
				if (child_obj->GetParent() == drag_drop_extra_check_parent)
				{
					drag_drop_extra_check_parent.clear();
					drag_drop_extra_check_child.clear();
				}

				child_obj->SetParent(parent_obj.get());
			}
		}

		if (check_range)
		{
			std::vector<std::string> keys;
			int origin = 0;
			int j = 0;
			for (const std::string& name : scene_objects)
			{
				j++;
				if (name == selected_obj->name)
				{
					origin = j;
				}
				keys.push_back(name);
			}
			
			//LOGDEBUGF("prev_index={} is_under={} cursor_index={} origin={}", prev_cursor_index, cursor_index_is_under, cursor_index, origin);
			cursor_index = std::clamp(cursor_index, (size_t)0, keys.size());

			selected_objects.clear();
			int dest = 0;
			if (cursor_index_is_under)
			{
				dest = static_cast<int>(cursor_index);
			}
			else
			{
				dest = static_cast<int>(cursor_index);
				origin -= 2;
			}
			for (int i = origin; cursor_index_is_under ? i < dest : i >= dest - 1; cursor_index_is_under ? i++ : i--)
			{
				LOGDEBUGF("{} {}", i, keys[i]);
				selected_objects.insert(keys[i]);
			}

			check_range = false;
		}

		bool hovering_scene_hierarchy = ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize());
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			if (!ImGui::IsAnyItemHovered() && hovering_scene_hierarchy)
			{
				selected_obj = nullptr;
				selected_objects.clear();
			}
		}

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			if (hovering_scene_hierarchy && ImGui::IsAnyItemHovered())
				hierarchy_clicked_object = true;
		}

		if (!ignore_mouse_click && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			if (hovering_scene_hierarchy)
			{
				if (!ImGui::IsPopupOpen("ScenePopup"))
				{
					ImGui::OpenPopup("ScenePopup");
				}
			}
		}

		if (ImGui::BeginPopup("ScenePopup"))
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Empty Object"))
					Scene::current_scene.AddToScene(Toad::Object("Object"), Toad::Engine::Get().GameStateIsPlaying());

				if (ImGui::MenuItem("Circle"))
					Scene::current_scene.AddToScene(Toad::Circle("Circle"), Toad::Engine::Get().GameStateIsPlaying());

				if (ImGui::MenuItem("Sprite"))
					Scene::current_scene.AddToScene(Toad::Sprite("Sprite"), Toad::Engine::Get().GameStateIsPlaying());

				if (ImGui::MenuItem("Audio"))
					Scene::current_scene.AddToScene(Toad::Audio("Audio"), Toad::Engine::Get().GameStateIsPlaying());

				if (ImGui::MenuItem("Text"))
					Scene::current_scene.AddToScene(Toad::Text("Text"), Toad::Engine::Get().GameStateIsPlaying());

				if (ImGui::MenuItem("Camera"))
				{
					Toad::Camera* cam = Scene::current_scene.AddToScene(Toad::Camera("Camera"), Toad::Engine::Get().GameStateIsPlaying()).get();
					cam->ActivateCamera();
				}

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		ImGui::PushOverrideID(scene_modify_popup_id);
		if (ImGui::BeginPopup("SceneModifyPopup"))
		{
			// multiple objects
			if (!selected_objects.empty())
			{
				if (ImGui::MenuItem("Delete"))
				{
					if (selected_obj != nullptr)
					{
						remove_objects_queue.push(selected_obj->name);
					}

					for (const std::string& name : selected_objects)
					{
						remove_objects_queue.push(name);
					}
				}
			}
			else
			{
				// singular object 
				if (ImGui::MenuItem("Delete"))
				{
					if (selected_obj != nullptr)
					{
						remove_objects_queue.push(selected_obj->name);
					}
				}
			}

			ImGui::EndPopup();
		}
		ImGui::PopID();

		while (!remove_objects_queue.empty())
		{
			const std::string& front = remove_objects_queue.front();
			selected_obj = nullptr;
			selected_objects.erase(front);
			Toad::Object* obj = Scene::current_scene.GetSceneObject(front).get();
			if (obj != nullptr)
			{
				obj->Destroy();
			}
			remove_objects_queue.pop();
		}

		ImGui::End();
	}

	ImGui::Begin("Inspector", nullptr);
	{
		if (hierarchy_clicked_object)
			inspector_ui = std::bind(&object_inspector, std::ref(selected_obj), asset_browser);

		if (inspector_ui)
			inspector_ui();
		
		ImGui::End();
	}

	ImGui::Begin("Game view", nullptr, ImGuiWindowFlags_HorizontalScrollbar);
	{
		auto content_size = ImGui::GetContentRegionAvail();
		// resize but keep aspect ratio
		constexpr float ar = 16.f / 9.f;
		float image_width = content_size.x;
		float image_height = content_size.x / ar;
		
		Toad::Camera* cam = Toad::Camera::GetActiveCamera();

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
			auto& window_texture = Toad::Engine::Get().GetWindowTexture();

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
				Toad::Engine::Get().viewport_size = { (int)image_width, (int)image_height };

				if (cam)
				{
					Toad::Engine::Get().interacting_camera = cam;
					Toad::Engine::Get().interacting_texture = &window_texture;
					float f_x = cam->GetSize().x / image_width;
					float f_y = cam->GetSize().y / image_height;

					Toad::Engine::Get().relative_mouse_pos = {
					(int)((ImGui::GetMousePos().x - pos.x) * f_x),
					(int)((ImGui::GetMousePos().y - pos.y) * f_y) };
				}
				else
					Toad::Engine::Get().interacting_camera = &Toad::Engine::Get().GetEditorCamera();
			}
		}

		ImGui::End();
	}
	
	bool viewport_opened = ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_HorizontalScrollbar);
	{
		auto& texture = Toad::Engine::Get().GetEditorCameraTexture();
		const auto content_size = ImGui::GetContentRegionAvail();

		Toad::Camera& editor_cam = Toad::Engine::Get().GetEditorCamera();

		static Vec2f initial_editor_cam_size = editor_cam.GetSize();

		constexpr float ar = 16.f / 9.f;
		float image_width = content_size.x;
		float image_height = content_size.x / ar;

		float fscale_x = image_width / editor_cam.GetSize().x;
		float fscale_y = image_height / editor_cam.GetSize().y;

		if (image_height > content_size.y)
		{
			image_height = content_size.y;
			image_width = content_size.y * ar;
		}

		float pady = 25; // #TODO: find the actual imgui style property 
		ImGui::SetCursorPos({
			(content_size.x - image_width) * 0.5f,
			(content_size.y - image_height + pady) * 0.5f
			});

		//editor_cam.SetSize({content_size.x, content_size.y});

		const auto pos = ImGui::GetCursorScreenPos();

		ImVec2 image_cursor_pos = ImGui::GetCursorPos();
		ImGui::Image(texture, { image_width, image_height }, sf::Color::White);

		if (ImGui::IsWindowHovered())
		{
			Toad::Engine::Get().interacting_camera = &editor_cam;
			Toad::Engine::Get().interacting_texture = &texture;

			Toad::Engine::Get().viewport_size = { (int)image_width, (int)image_height };
			float f_x = editor_cam.GetSize().x / image_width;
			float f_y = editor_cam.GetSize().y / image_height;

			Toad::Engine::Get().relative_mouse_pos = {
				(int)((ImGui::GetMousePos().x - pos.x) * f_x),
				(int)((ImGui::GetMousePos().y - pos.y) * f_y) };
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dnd tilesheettile"))
			{
				SheetTileData data = *(SheetTileData*)payload->Data;

				sf::Texture* stex = ResourceManager::GetTextures().Get(data.path);
				if (!stex)
				{
					LOGERRORF("[UI:Viewport Drag Drop tilesheet] texture can't be loaded from resourcemanager: {}", data.path);
				}
				else
				{
					Toad::Sprite* added_obj = Scene::current_scene.AddToScene(Toad::Sprite("Sprite"), Toad::Engine::Get().GameStateIsPlaying()).get();

					added_obj->SetTexture(data.path, stex);
					added_obj->GetSprite().setTextureRect(data.tex_rect);
					added_obj->GetSprite().setScale(data.tex_size);
					ImVec2 curr_pos = { ImGui::GetMousePos().x - pos.x, ImGui::GetMousePos().y - pos.y };
					float fx = editor_cam.GetSize().x / image_width;
					float fy = editor_cam.GetSize().y / image_height;
					Vec2f pos = { curr_pos.x * fx, curr_pos.y * fy };

					pos = texture.mapPixelToCoords({(int)pos.x, (int)pos.y}, editor_cam.GetView());

					added_obj->SetPosition(pos);
				}
			}

			ImGui::EndDragDropTarget();
		}
		ImGui::GetWindowDrawList()->AddText({ pos.x + content_size.x - 70, pos.y + 10 }, IM_COL32_WHITE, Toad::format_str("CW:{}\nCH:{}", content_size.x, content_size.y).c_str());
		ImGui::GetWindowDrawList()->AddText({ pos.x + content_size.x - 70, pos.y + 40 }, IM_COL32_WHITE, Toad::format_str("IW:{}\nIH:{}", image_width, image_height).c_str());

		std::vector<ImVec2> positions;
		static bool always_show_object_names = true;

		const auto obj_screen_pos_info = [&](Toad::Object* obj, bool add_to_list = true)
			{
				if (obj)
				{
					Vec2i obj_pos_px = texture.mapCoordsToPixel(obj->GetPosition(), editor_cam.GetView());
					Vec2f obj_pos_px_flt = { (float)obj_pos_px.x, (float)obj_pos_px.y };

					float scale_x = image_width / initial_editor_cam_size.x;
					float scale_y = image_height / initial_editor_cam_size.y;

					obj_pos_px_flt.x *= scale_x;
					obj_pos_px_flt.y *= scale_y;
					obj_pos_px_flt.x += pos.x;
					obj_pos_px_flt.y += pos.y;

					if (add_to_list)
					{
						positions.emplace_back(obj_pos_px_flt.x, obj_pos_px_flt.y);
						ImGui::GetWindowDrawList()->AddText(ImVec2{ obj_pos_px_flt.x, obj_pos_px_flt.y }, IM_COL32(255, 255, 0, 160), obj->name.c_str());
					}
					else
					{
						ImGui::GetWindowDrawList()->AddText(ImVec2{ obj_pos_px_flt.x, obj_pos_px_flt.y }, IM_COL32(255, 255, 0, 100), obj->name.c_str());
					}
				}
			};

		if (always_show_object_names)
		{
			for (const auto& obj : Scene::current_scene.objects_all)
			{
				if (obj.get() == selected_obj || selected_objects.contains(obj->name))
				{
					obj_screen_pos_info(selected_obj);
				}
				else
				{
					obj_screen_pos_info(obj.get(), false);
				}
			}
		}
		if (selected_obj)
		{
			obj_screen_pos_info(selected_obj);
		}
		for (const auto& name : selected_objects)
		{
			Toad::Object* obj = Scene::current_scene.GetSceneObject(name).get();
			if (obj)
			{
				obj_screen_pos_info(obj);
			}
		}

		// #TODO abstract gizmo 
		static ImVec2 select_begin_relative = {};
		static ImVec2 select_begin_cursor = {};
		static bool is_moving_gizmo = false;

		ImVec2 gizmo_pos;
		static Vec2i selected_gizmo = { 0, 0 }; // #TODO enum 
		static ImU32 gizmo_col_xy = ImGui::ColorConvertFloat4ToU32({ 1, 0, 0, 0.5f });
		static ImU32 gizmo_col_x = ImGui::ColorConvertFloat4ToU32({ 0, 0, 1, 0 });
		static ImU32 gizmo_col_y = ImGui::ColorConvertFloat4ToU32({ 0, 1, 0, 0 });
		
		ImVec2 gizmo_xy_size = { 10, 10 };
		const float gizmo_line_size = 40;
		const float gizmo_line_width = 2.f;

		for (const ImVec2& position : positions)
		{
			gizmo_pos.x += position.x;
			gizmo_pos.y += position.y;
		}

		gizmo_pos.x /= positions.size();
		gizmo_pos.y /= positions.size();

		ImGui::GetWindowDrawList()->AddRectFilled(gizmo_pos - gizmo_xy_size, gizmo_pos + gizmo_xy_size, gizmo_col_xy);
		// y
		ImGui::GetWindowDrawList()->AddLine(gizmo_pos, {gizmo_pos.x, gizmo_pos.y + gizmo_line_size}, gizmo_col_y, gizmo_line_width);
		// x
		ImGui::GetWindowDrawList()->AddLine(gizmo_pos, {gizmo_pos.x + gizmo_line_size, gizmo_pos.y}, gizmo_col_x, gizmo_line_width);

		// draw grid 
		//for (int i = 0; i < content_size.x / grid_size.x; i++)
		{
			//ImGui::GetWindowDrawList()->AddLine( pos + ImVec2{i * grid_size.x, 0}, pos + ImVec2{i * grid_size.x, content_size.y}, IM_COL32_WHITE);
		}

		if (is_moving_gizmo)
		{
			ImVec2 d = ImGui::GetMouseDragDelta(0, 0.f);

			static bool temp_disable_snapping = false;
			if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && drag_snap)
			{
				temp_disable_snapping = true;
				drag_snap = false;
			}

			if (ImGui::IsKeyReleased(ImGuiKey_LeftShift) && temp_disable_snapping)
			{
				temp_disable_snapping = false;
				drag_snap = true;
			}
				
			if (ImGui::IsKeyPressed(ImGuiKey_X))
			{
				selected_gizmo.x = ~selected_gizmo.x;
				if (selected_gizmo.y)
				{
					selected_gizmo.y = ~selected_gizmo.y;
				}
				if (selected_gizmo.x)
				{
					gizmo_col_x = ImGui::ColorConvertFloat4ToU32({ 0, 0, 1, 0.8f });
					gizmo_col_y = ImGui::ColorConvertFloat4ToU32({ 0, 0, 1, 0 });
				}
				else
				{
					gizmo_col_x = ImGui::ColorConvertFloat4ToU32({ 0, 0, 1, 0 });
				}
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_Y))
			{
				selected_gizmo.y = ~selected_gizmo.y;
				if (selected_gizmo.x)
				{
					selected_gizmo.x = ~selected_gizmo.x;
				}
				if (selected_gizmo.y)
				{
					gizmo_col_y = ImGui::ColorConvertFloat4ToU32({ 0, 1, 0, 0.8f });
					gizmo_col_x = ImGui::ColorConvertFloat4ToU32({ 0, 0, 1, 0 });

				}
				else
				{
					gizmo_col_y = ImGui::ColorConvertFloat4ToU32({ 0, 1, 0, 0 });
				}
			}

			if (d.x || d.y)
			{
				if (selected_gizmo.x)
					d.y = 0;
				else if (selected_gizmo.y)
					d.x = 0;

				float multiplierx = editor_cam.GetSize().x / image_width;
				float multipliery = editor_cam.GetSize().y / image_height;

				d.x *= multiplierx;
				d.y *= multipliery;

				if (selected_obj)
				{
					if (drag_snap)
					{
						Vec2f drag_pos = selected_obj->GetPosition() + Vec2f{ d.x, d.y };

						const Vec2i& mouse_pos = Toad::Engine::Get().relative_mouse_pos;
						Vec2f mouse_world_pos = Toad::Screen::ScreenToWorld(mouse_pos);
						
						// snap to the grid
						mouse_world_pos.x = std::round(mouse_world_pos.x / grid_size.x) * grid_size.x;
						mouse_world_pos.y = std::round(mouse_world_pos.y / grid_size.y) * grid_size.y;

						selected_obj->SetPosition(mouse_world_pos);	

						FloatRect selected_obj_bounds = selected_obj->GetBounds();

						if (selected_obj_bounds.width != 0 && selected_obj_bounds.height != 0)
						{
							for (const auto& obj : Scene::current_scene.objects_all)
							{
								if (selected_obj == obj.get())
									continue;

								FloatRect bounds = obj->GetBounds();
								if (bounds.width == 0 || bounds.height == 0)
									continue;
								
								bounds.Expand(5.f);
								if (bounds.Contains(mouse_world_pos))
								{
									// snap to nearest edge
									float right = bounds.left + bounds.width;
									float bottom = bounds.top + bounds.height;

									if (mouse_world_pos.x > right)
									{

									}
								}
							}
						}
					}
					else
					{
						selected_obj->SetPosition(selected_obj->GetPosition() + Vec2f{ d.x, d.y });
					}
				}
				for (const std::string& name : selected_objects)
				{
					Toad::Object* obj = Scene::current_scene.GetSceneObject(name).get();
					if (obj)
						obj->SetPosition(obj->GetPosition() + Vec2f{ d.x, d.y });
				}

				ImGuiContext* g = ImGui::GetCurrentContext();
				*g->IO.MouseClickedPos = ImGui::GetMousePos();
			}
			if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				selected_gizmo = { 0, 0 };
				gizmo_col_x = ImGui::ColorConvertFloat4ToU32({ 0, 0, 0, 0 });
				gizmo_col_y = ImGui::ColorConvertFloat4ToU32({ 0, 0, 0, 0 });
				is_moving_gizmo = false;
			}
		}
		else
		{	
			// xy 
			if (ImRect((gizmo_pos - gizmo_xy_size), gizmo_pos + gizmo_xy_size).Contains(ImGui::GetMousePos()))
			{
				gizmo_col_xy = ImGui::ColorConvertFloat4ToU32({ 1, 0, 0, 1 });
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					is_moving_gizmo = true;
				else
					is_moving_gizmo = false;
			}
			else
			{
				gizmo_col_xy = ImGui::ColorConvertFloat4ToU32({ 1, 0, 0, 0.5f });
			}
		}

		if (viewport_opened)
		{
			if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
			{
				ImGuiContext* g = ImGui::GetCurrentContext();
				ImVec2 d = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle, 0.f);
				g->IO.MouseClickedPos[ImGuiMouseButton_Middle] = ImGui::GetMousePos();
				editor_cam.SetPosition(editor_cam.GetPosition() - Vec2f(d.x, d.y));
			}

			float mwheel = ImGui::GetIO().MouseWheel;
			if (mwheel)
			{
				if (mwheel < 0)
				{
					editor_cam.Zoom(1.2f);
				}
				else
				{
					editor_cam.Zoom(0.8f);
				}
			}
		}

		if (ImGui::IsItemHovered())
		{
			if (!is_moving_gizmo)
			{
				if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
				{
					ImGui::GetWindowDrawList()->AddRectFilled(select_begin_cursor, select_begin_cursor + ImGui::GetMouseDragDelta(), IM_COL32(155, 155, 255, 50), 0);
					auto texture_size = texture.getSize();

					ImVec2 curr_pos = { ImGui::GetMousePos().x - pos.x, ImGui::GetMousePos().y - pos.y };

					//float fx = editor_cam.GetSize().x / image_width;
					//float fy = editor_cam.GetSize().y / image_height;
					float x1 = select_begin_relative.x + pos.x;
					float y1 = select_begin_relative.y + pos.y;
					float x2 = curr_pos.x + pos.x;
					float y2 = curr_pos.y + pos.y;

					//LOGDEBUGF("{} {}  {} {}", x1, y1, x2, y2);
					
					ImRect rect(std::min(x1, x2), std::min(y1, y2), std::max(x1, x2), std::max(y1, y2));
					
					for (const auto& obj : Scene::current_scene.objects_all)
					{
						Vec2i a = texture.mapCoordsToPixel(obj->GetPosition(), editor_cam.GetView());
						Vec2f a_flt = { (float)a.x, (float)a.y };

						float scale_x = image_width / initial_editor_cam_size.x;
						float scale_y = image_height / initial_editor_cam_size.y;

						a_flt.x *= scale_x;
						a_flt.y *= scale_y;
						a_flt.x += pos.x;
						a_flt.y += pos.y;

						if (rect.Contains({ a_flt.x, a_flt.y })) {
							if (selected_obj != obj.get())
								selected_objects.emplace(obj->name);
							else
								selected_obj = obj.get();
						}
						else
						{
							if (!ImGui::IsKeyDown(ImGuiKey_LeftShift))
							{
								if (selected_obj == obj.get())
									selected_obj = nullptr;
								else if (selected_objects.contains(obj->name))
									selected_objects.erase(obj->name);
							}
						}
					}
				}
				else if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
				{
					//LOGDEBUGF("{} {} {} {}", ImGui::GetMousePos().x, ImGui::GetMousePos().y, image_cursor_pos.x, image_cursor_pos.y);
					select_begin_cursor = ImGui::GetMousePos();
					select_begin_relative = { ImGui::GetMousePos().x - pos.x, ImGui::GetMousePos().y - pos.y };

					//if (!ImGui::IsAnyItemHovered())
					//{
					//	selected_obj = nullptr;
					//	selected_objects.clear();
					//}
				}

				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					// only selected one object
					if (selected_objects.size() == 1 && !selected_obj)
					{
						selected_obj = Toad::Scene::current_scene.GetSceneObject(*selected_objects.begin()).get();
						selected_objects.erase(selected_objects.begin());
					}
					
					inspector_ui = std::bind(&object_inspector, std::ref(selected_obj), asset_browser);
				}
			}
		}

		ImGui::SetCursorPos({ ImGui::GetScrollX() + 20, 20 });
		if (ImGui::TreeNode("Viewport Options"))
		{
			static fs::path last_scene_path;

			if (!Toad::Engine::Get().GameStateIsPlaying())
			{
				if (ImGui::Button("Play"))
				{
					if (reload_scene_on_stop)
						last_scene_path = Scene::current_scene.path;

					Toad::Engine::Get().StartGameSession();
				}
			}
			else
			{
				if (ImGui::Button("Stop"))
				{
					if (reload_scene_on_stop)
						Scene::current_scene = LoadScene(last_scene_path, asset_browser.GetAssetPath());

					Toad::Engine::Get().StopGameSession();
				}
			}

			static int fps = 60;
			static bool fps_unlocked = false;
			ImGui::BeginDisabled(fps_unlocked);
			if (ImGui::DragInt("FPS", &fps, 1, 10, 100000))
			{
				Toad::Engine::Get().GetWindow().setFramerateLimit(std::clamp(fps, 10, 100000));
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Checkbox("Unlock", &fps_unlocked))
			{
				if (fps_unlocked)
				{
					Toad::Engine::Get().GetWindow().setFramerateLimit(0);
				}
				else
				{
					Toad::Engine::Get().GetWindow().setFramerateLimit(fps);
				}
			}

			ImGui::Checkbox("Reload scene on stop", &reload_scene_on_stop);

			if (ImGui::TreeNode("Editor Camera Settings"))
			{
				Vec2f pos = editor_cam.GetPosition();
				Vec2f size = editor_cam.GetSize();
				if (ImGui::SliderVec2("pos", &pos, FLT_MIN, FLT_MAX))
					editor_cam.SetPosition(pos);
				if (ImGui::SliderVec2("size", &size, FLT_MIN, FLT_MAX))
					editor_cam.SetSize(size);
				ImGui::TreePop();
			}

			ImGui::Checkbox("Show grid", &show_grid);
			ImGui::Checkbox("Snapping", &drag_snap);
			ImGui::SliderVec2i("Grid", &grid_size);

			ImGui::TreePop();
		}

		static bool is_animator = false;
		static std::vector<TileSpritePlacer> opened_tilemaps;
		if (ImGui::TreeNode("Tools"))
		{
			ImGui::Checkbox("animator", &is_animator);
			if (ImGui::TreeNode("open tilemaps"))
			{
				ImGui::Button("open (draggable by file)");
				if (ImGui::BeginDragDropTarget())
				{
					const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("move file");
					if (payload != nullptr)
					{
						std::filesystem::path src = *(std::string*)payload->Data;
						do
						{
							if (!src.has_extension() || (src.extension().string() != ".jpg" && src.extension().string() != ".png"))
							{
								LOGERRORF("[UI:Tools:open tilemaps] {} is not a valid file for a tilemap", src);
								break;
							}
							for (const auto& t : opened_tilemaps)
							{
								if (t.path == src)
								{
									LOGDEBUGF("[UI:Tools:open tilemaps] {} already is an existing tilemap", src);
									break;
								}
							}

							sf::Texture tex;
							if (!tex.loadFromFile(src.string()))
							{
								LOGDEBUGF("[UI:Tools:open tilemaps] {} can't be opened/loaded", src);
								break;
							}
							opened_tilemaps.push_back(TileSpritePlacer(src, { 16, 16 }, tex, {}));
						} while (false);
					}

					ImGui::EndDragDropTarget();
				}
				for (auto it = opened_tilemaps.begin(); it != opened_tilemaps.end();)
				{
					ImGui::Text("%s", it->path.filename().string().c_str());
					ImGui::SameLine();
					if (ImGui::Button("close"))
					{
						it = opened_tilemaps.erase(it);
					}
					else
					{
						++it;
					}
				}

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		for (auto& t : opened_tilemaps)
		{
			ImGui::Begin(t.path.filename().string().c_str());
			{
				static bool ignore_fully_transparent = true;
				static bool preview_size = true;
				static Vec2f tile_size = {1.f, 1.f};
				ImGui::SliderVec2i("split size", &t.size);
				ImGui::SliderVec2("tile dropped size", &tile_size);

				ImGui::Checkbox("ignore full transparent tiles", &ignore_fully_transparent);
				ImGui::Checkbox("preview size", &preview_size);
				if (ImGui::Button("APPLY SIZE/RESET TEXTURE"))
				{
					assert(t.size.x < t.tile_map.getSize().x && t.size.y < t.tile_map.getSize().y);

					t.tiles.clear();

					for (uint32_t i = 0; i < t.tile_map.getSize().x; i += t.size.x)
					{
						for (uint32_t j = 0; j < t.tile_map.getSize().y; j += t.size.y)
						{
							sf::Sprite sprite = sf::Sprite(t.tile_map);
							sprite.setTextureRect(sf::IntRect({ (int)i, (int)j }, t.size));

							bool skip = false;

							if (ignore_fully_transparent)
							{
								skip = true;
								// must copy to an image 
								auto image = t.tile_map.copyToImage();

								for (uint32_t sprite_x = i; sprite_x < i + t.size.x; sprite_x++)
								{
									for (uint32_t sprite_y = j; sprite_y < j + t.size.y; sprite_y++)
									{
										if (image.getPixel(sprite_x, sprite_y) != sf::Color::Transparent)
										{
											skip = false;
										}
									}
								}
							}
							
							if (!skip)
							{
								t.tiles.emplace_back(sprite);
							}
						}
					}
				}
				if (ImGui::TreeNode("preview"))
				{
					t.size.x = std::clamp(t.size.x, 5, INT_MAX);
					t.size.y = std::clamp(t.size.y, 5, INT_MAX);

					ImVec2 screen_pos = ImGui::GetCursorScreenPos();
					ImGui::Image(t.tile_map);
					if (preview_size)
					{
						auto draw = ImGui::GetWindowDrawList();
						for (uint32_t x = 0; x < t.tile_map.getSize().x; x += t.size.x)
						{
							for (uint32_t y = 0; y < t.tile_map.getSize().y; y += t.size.y)
							{
								draw->AddLine(
									screen_pos + ImVec2{ (float)x, (float)y },
									screen_pos + ImVec2{ (float)x, (float)t.tile_map.getSize().y},
									IM_COL32(255, 255, 255, 60)
								);
								draw->AddLine(
									screen_pos + ImVec2{ (float)x, (float)y }, 
									screen_pos + ImVec2{ (float)t.tile_map.getSize().x, (float)y },
									IM_COL32(255, 255, 255, 60)
								);
							}
						}
					}
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("drag & drop"))
				{
					float y = ImGui::GetCursorPosY();
					uint32_t total_h = 0;
					
					// t.tiles is saved vertically 
					// split them up and store them in vertical lines for proper display
					std::vector<std::vector<const sf::Sprite*>> splitted_tiles;
					std::vector <const sf::Sprite*> line;

					for (const auto& t2 : t.tiles)
					{
						total_h += t.size.y;
						line.push_back(&t2);
						if (total_h > t.tile_map.getSize().y)
						{
							splitted_tiles.push_back(line);
							line.clear();
							total_h = 0;
						}
					}
					for (uint32_t i = 0; i < splitted_tiles.size(); i++)
					{
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });
						ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });
						ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
						ImGui::BeginChild(Toad::format_str("{}", i).c_str(), {(float)t.size.x, ImGui::GetWindowHeight() - y}, false, ImGuiWindowFlags_NoScrollbar);
						for (const sf::Sprite* sprite : splitted_tiles[i])
						{
							ImGui::Image(*sprite);
							if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
							{
								std::filesystem::path relative = std::filesystem::relative(t.path, asset_browser.GetAssetPath());

								if (!ResourceManager::GetTextures().Get(relative.string()))
								{
									ResourceManager::GetTextures().Add(relative.string(), t.tile_map);
								}

								SheetTileData data;
								data.path = new char[relative.string().length()];
								strncpy(data.path, relative.string().c_str(), relative.string().length() + 1);
								data.tex_rect = sprite->getTextureRect();
								data.tex_size = tile_size;
								ImGui::SetDragDropPayload("dnd tilesheettile", &data, sizeof(data));
								ImGui::EndDragDropSource();
							}

						}
						ImGui::EndChild();
						ImGui::PopStyleVar(3);

						if (i < splitted_tiles.size() - 1)
						{
							ImGui::SameLine(0, 0);
						}
					}

					ImGui::TreePop();
				}
				ImGui::End();
			}
		}

		static Toad::Object* selected_animation_obj = nullptr;
		static Toad::Animation anim;
		if (is_animator)
		{
			anim_editor.Show(&is_animator);
		}
			//ImGui::SetCursorPosY(pos.y + image_height);
		//	ImGui::Begin("Anim Editor", nullptr, ImGuiWindowFlags_NoBackground);
		//	{
		//		if (selected_animation_obj)
		//		{
		//			static int timeline = 0;
		//			bool update_all_frames = false;
		//			static int anim_length = anim.frames.size();
		//			
		//			int spacex = anim_length / ImGui::GetWindowSize().x;
		//			int cursorposx = ImGui::GetCursorPosX();

		//			ImGui::PushItemWidth(ImGui::GetWindowSize().x);
		//			ImGui::SliderInt("time", &timeline, 0, anim_length - 1);

		//			Toad::AnimationFrame& current_frame = anim.frames[timeline];

		//			ImGui::BeginDisabled(current_frame.is_key);
		//			if (ImGui::Button("KEY ADD"))
		//			{
		//				current_frame.is_key = true;
		//			}
		//			ImGui::EndDisabled();
		//			ImGui::BeginDisabled(!current_frame.is_key);
		//			ImGui::SameLine();
		//			if (ImGui::Button("KEY REMOVE"))
		//			{
		//				current_frame.is_key = false;
		//			}
		//			ImGui::EndDisabled();
		//			ImGui::SameLine();
		//			if (ImGui::Button("FRAMES UPDATE"))
		//			{
		//				update_all_frames = true;
		//			}
		//			ImGui::SameLine();
		//			if (ImGui::Button("SET LENGTH.."))
		//			{
		//				anim_length = anim.frames.size();
		//				ImGui::OpenPopup("AnimLengthPopUp");
		//			}
		//			ImGui::SameLine();
		//			if (ImGui::Button("SAVE.."))
		//			{

		//			}

		//			if (ImGui::BeginPopupModal("AnimLengthPopUp", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		//			{
		//				static int new_size = anim_length; 
		//				ImGui::DragInt("size", &new_size);
		//				if (new_size < anim_length)
		//				{
		//					ImGui::TextColored({ 1, 1, 0, 1 }, "Will remove old frames: new:%d < old:%d", new_size, anim_length);
		//				}
		//				if (ImGui::Button("ok"))
		//				{
		//					anim.frames.resize((size_t)new_size);
		//					anim_length = new_size;
		//					ImGui::CloseCurrentPopup();
		//				}
		//				ImGui::EndPopup();
		//			}
		//			
		//			ImGui::BeginChild("properties", {0, 100}, true);
		//			{
		//				ImGui::SliderVec2("position", &current_frame.position);
		//				ImGui::SliderVec2("scale   ", &current_frame.scale);
		//				ImGui::DragFloat("rotation", &current_frame.rotation);

		//				ImGui::EndChild();
		//			}
		//			if (update_all_frames)
		//			{
		//				// interpolation between key frames 

		//				for (int i = 0; i < anim.frames.size(); i++)
		//				{
		//					int start = 0;
		//					int end = 0;

		//					if (anim.frames[i].is_key || i == 0)
		//					{
		//						start = i;
		//						for (int j = i + 1; j < anim.frames.size(); j++)
		//						{
		//							if (anim.frames[j].is_key)
		//							{
		//								end = j;
		//								for (int k = start; k < end; k++)
		//								{
		//									float t = (float)(k - start) / (float)(end - start);
		//									float posx = std::lerp(anim.frames[i].position.x, anim.frames[j].position.x, t);
		//									float posy = std::lerp(anim.frames[i].position.y, anim.frames[j].position.y, t);
		//									float scalex = std::lerp(anim.frames[i].scale.x, anim.frames[j].scale.x, t);
		//									float scaley = std::lerp(anim.frames[i].scale.y, anim.frames[j].scale.y, t);
		//									float rotation = std::lerp(anim.frames[i].rotation, anim.frames[j].rotation, t);

		//									anim.frames[k].position = { posx, posy };
		//									anim.frames[k].scale = { scalex, scaley };
		//									anim.frames[k].rotation = rotation;
		//								}

		//								start = end;
		//								i = start;
		//							}
		//						}
		//					}
		//				}
		//			}
		//			for (int i = 0; i < anim.frames.size(); i++)
		//			{
		//				Toad::AnimationFrame& frame = anim.frames[i];

		//				cursorposx += spacex * 2 + ImGui::GetStyle().FramePadding.x;
		//				if (frame.is_key)
		//				{
		//					ImGui::SetCursorPosX(cursorposx);
		//					if (timeline == i)
		//					{
		//						ImGui::TextColored({ 1, 1, 1, 1, }, "O");
		//					}
		//					else
		//					{
		//						ImGui::TextColored({ 1, 1, 1, 0.6f, }, "O");
		//					}
		//					cursorposx += ImGui::CalcTextSize("O").x;
		//					ImGui::SameLine();
		//				}
		//				else 
		//				{
		//					ImGui::SetCursorPosX(cursorposx);
		//					if (timeline == i)
		//					{
		//						ImGui::TextColored({ 1, 1, 1, 1, }, "_");
		//					}
		//					else
		//					{
		//						ImGui::TextColored({ 1, 1, 1, 0.6f, }, "_");
		//					}
		//					cursorposx += ImGui::CalcTextSize("_").x;
		//					ImGui::SameLine();
		//				}

		//				ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		//				ImGui::SameLine();
		//			}
		//			ImGui::PopItemWidth();
		//		}
		//		else
		//		{
		//			ImGui::TextColored({ 1, 1, 0, 1 }, "Select an object in the scene to animate");

		//			char input[256] = "";
		//			std::vector<std::pair<std::string, Toad::Object*>> matches;
		//			auto& scene = Toad::Engine::Get().GetScene();
		//			matches.reserve(scene.objects_all.size());

		//			if (ImGui::InputText("search", input, 256))
		//			{
		//				for (const auto& obj : scene.objects_all)
		//				{
		//					if (obj->name.find(input) != std::string::npos)
		//					{
		//						matches.emplace_back(obj->name, obj.get());
		//					}
		//				}
		//			}

		//			if (strlen(input) == 0)
		//			{
		//				for (const auto& obj : scene.objects_all)
		//				{
		//					matches.emplace_back(obj->name, obj.get());
		//				}
		//			}

		//			static Toad::Object* selected = nullptr;
		//			for (const auto& [name, obj] : matches)
		//			{
		//				if (ImGui::Selectable(name.c_str(), selected == obj))
		//				{
		//					selected = obj;
		//				}
		//				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		//				{
		//					selected_animation_obj = selected;
		//					anim.frames.resize(10);
		//					selected = nullptr;
		//				}
		//			}
		//		}
		//		ImGui::End();
		//	}
		//}
		//

		ImGui::End();
	}

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
						Scene::SetScene(&Toad::LoadScene(file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
						scene_history.asset_folder = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
						scene_history.scene = &Scene::current_scene;
						if (scene_history.SaveState())
							saved_scene = false;
					}
					else
					{
						Scene::SetScene(&Toad::LoadScene(file, asset_browser.GetAssetPath()));
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
			textEditor.Show(fBrowser.GetSelectedFileContent());
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
				if (Toad::Engine::Get().GameStateIsPlaying())
					Toad::Engine::Get().StopGameSession();

				selected_obj = nullptr;
				selected_objects.clear();
				scene_history.Redo();
			}
			else
			{
				if (Toad::Engine::Get().GameStateIsPlaying())
					Toad::Engine::Get().StopGameSession();

				selected_obj = nullptr;
				selected_objects.clear();
				scene_history.Undo();
			}
		}
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
		break;
	default: 
		break;
	}
}

void ui::editor_texture_draw_callback(sf::RenderTexture& texture)
{
	const Toad::Camera* active_cam = Toad::Camera::GetActiveCamera();
	if (!show_grid || !active_cam)
	{
		return;
	}
	const Toad::Camera& cam = Toad::Engine::Get().GetEditorCamera();
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

void ui::object_inspector(Toad::Object*& selected_obj, const Toad::GameAssetsBrowser& asset_browser)
{
	if (selected_obj != nullptr)
	{
		const auto& attached_scripts = selected_obj->GetAttachedScripts();
		bool suggestion = false;
		char name_buf[100];
		std::string new_name_str;
		strncpy(name_buf, selected_obj->name.c_str(), selected_obj->name.length() + 1);

		ImVec2 input_name_pos = ImGui::GetCursorPos();
		if (ImGui::InputText("name", name_buf, sizeof(name_buf)))
		{
			Toad::Scene& scene = Scene::current_scene;

			new_name_str = name_buf;

			int count = 0;
			bool found = false;

			for (const auto& obj : scene.objects_all)
			{
				if (obj->name == new_name_str)
				{
					count++;
					found = true;
				}
			}

			if (new_name_str != selected_obj->name && found)
			{
				suggestion = true;

				new_name_str += " (" + std::to_string(count) + ')';
				auto it = std::ranges::find_if(scene.objects_all, [&new_name_str](const std::shared_ptr<Toad::Object>& obj) {return obj->name == new_name_str; });
				while (it != scene.objects_all.end())
				{
					new_name_str += " (" + std::to_string(++count) + ')';
					it = std::ranges::find_if(scene.objects_all, [&new_name_str](const std::shared_ptr<Toad::Object>& obj) {return obj->name == new_name_str; });
				}
			}
			else
			{
				suggestion = false;
			}
		}

		if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
		{
			if (new_name_str != selected_obj->name)
			{
				selected_obj->name = new_name_str;

				/*auto& map = Scene::current_scene.objects_map;
				if (!map.contains(new_name_str))
				{
					auto a = map.extract(selected_obj->name);
					a.key() = new_name_str;
					map.insert(std::move(a));

					selected_obj->name = new_name_str;
				}*/
			}
		}

		if (suggestion)
		{
			//ImGuiInputTextState* state = ImGui::GetInputTextState(ImGui::GetID("name"));
			std::string part_of_str = new_name_str.substr(strlen(name_buf), new_name_str.length() - strlen(name_buf));
			ImVec2 frame_pad = ImGui::GetStyle().FramePadding;
			ImVec2 text_size = ImGui::CalcTextSize(name_buf);
			ImGui::SetCursorPos({ input_name_pos.x + text_size.x + frame_pad.x, input_name_pos.y + frame_pad.y });
			ImGui::TextColored({ 1,1,1,0.4f }, "%s", part_of_str.c_str());
		}

		ImGui::Text("%s", selected_obj->name.c_str());

		static Toad::Object* prev_selected_obj = nullptr;
		static size_t index = 0;
		auto& obj_all = Scene::current_scene.objects_all;
		if (prev_selected_obj != selected_obj)
		{
			for (size_t i = 0; i < obj_all.size(); i++)
			{
				if (obj_all[i].get() == selected_obj)
				{
					index = i;
				}
			}
		}

		prev_selected_obj = selected_obj;

		ImGui::SameLine(0, 10.f);
		ImGui::BeginDisabled(index == 0);
		if (ImGui::ArrowButton("##MoveUp", ImGuiDir_Up))
		{
			std::swap(obj_all[index], obj_all[index - 1]);
			--index;
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(index == obj_all.size() - 1);
		if (ImGui::ArrowButton("##MoveDown", ImGuiDir_Down))
		{
			std::swap(obj_all[index], obj_all[index + 1]);
			++index;
		}
		ImGui::EndDisabled();

		float pos_x = selected_obj->GetPosition().x;
		float pos_y = selected_obj->GetPosition().y;
		float rotation = selected_obj->GetRotation();

		if (ImGui::DragFloat("x", &pos_x))
		{
			selected_obj->SetPosition({ pos_x, pos_y });
		}
		if (ImGui::DragFloat("y", &pos_y))
		{
			selected_obj->SetPosition({ pos_x, pos_y });
		}
		if (ImGui::DragFloat("rotation (degrees)", &rotation))
		{
			selected_obj->SetRotation(rotation);
		}

		//if (ImGui::Button("Test"))
		//{
		//	auto child_obj = Scene::current_scene.AddToScene(Toad::Circle("child object"));
		//	child_obj->SetParent(selected_obj);
		//}

		ImGui::SeparatorText("object properties");

		auto sprite_obj = dynamic_cast<Toad::Sprite*>(selected_obj);
		auto circle_obj = dynamic_cast<Toad::Circle*>(selected_obj);
		auto audio_obj = dynamic_cast<Toad::Audio*>(selected_obj);
		auto text_obj = dynamic_cast<Toad::Text*>(selected_obj);
		auto cam_obj = dynamic_cast<Toad::Camera*>(selected_obj);

		if (sprite_obj != nullptr)
		{
			auto& sprite = sprite_obj->GetSprite();

			const sf::Texture* attached_texture = sprite.getTexture();

			ImGui::Text("texture");
			ImGui::SameLine();

			if (attached_texture != nullptr)
			{
				if (ImGui::ImageButton(*attached_texture, { 25, 25 }))
				{
					// TODO:
				}
			}
			else
			{
				if (ImGui::Button("", { 25, 25 }))
				{

				}
			}

			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("move file");
				if (payload != nullptr)
				{
					std::filesystem::path src = *(std::string*)payload->Data;
					do
					{
						if (!src.has_extension() || (src.extension().string() != ".jpg" && src.extension().string() != ".png"))
						{
							break;
						}
						
						std::filesystem::path relative = std::filesystem::relative(src, asset_browser.GetAssetPath());
						if (ResourceManager::GetTextures().GetData().contains(relative.string()))
						{
							sf::Texture* managed_texture = ResourceManager::GetTextures().Get(relative.string());
							sprite_obj->SetTexture(relative, managed_texture);
						}
						else
						{
							sf::Texture new_texture;
							if (!new_texture.loadFromFile(src.string()))
							{
								LOGERRORF("Failed to load texture from {}", src);
							}
							sf::Texture* managed_texture = ResourceManager::GetTextures().Add(relative.string(), new_texture);
							if (managed_texture != nullptr)
							{
								sprite_obj->SetTexture(relative, managed_texture);
							}
						}

					} while (false);
				}

				ImGui::EndDragDropTarget();
			}

			if (attached_texture != nullptr)
			{
				if (ImGui::TreeNode("texture rect"))
				{
					sf::IntRect rect = sprite.getTextureRect();
					if (ImGui::DragInt("left", &rect.left))
					{
						sprite.setTextureRect(rect);
					}
					if (ImGui::DragInt("top", &rect.top))
					{
						sprite.setTextureRect(rect);
					}
					if (ImGui::DragInt("width", &rect.width))
					{
						sprite.setTextureRect(rect);
					}
					if (ImGui::DragInt("height", &rect.height))
					{
						sprite.setTextureRect(rect);
					}

					ImGui::TreePop();
				}
			}

			const sf::Color& sprite_col = sprite.getColor();
			float col[4] = {
				sprite_col.r / 255.f,
				sprite_col.g / 255.f,
				sprite_col.b / 255.f,
				sprite_col.a / 255.f
			};
			if (ImGui::ColorEdit4("color", col))
				sprite.setColor(sf::Color{
				(uint8_t)(col[0] * 255.f),
				(uint8_t)(col[1] * 255.f),
				(uint8_t)(col[2] * 255.f), 
				(uint8_t)(col[3] * 255.f)});

			Vec2f scale = sprite.getScale();
			if (ImGui::DragFloat("scale x", &scale.x))
			{
				sprite.setScale(scale);
			}
			if (ImGui::DragFloat("scale y", &scale.y))
			{
				sprite.setScale(scale);
			}

			Vec2f origin = sprite.getOrigin();
			if (ImGui::DragFloat("origin x", &origin.x))
			{
				sprite.setOrigin(origin);
			}
			if (ImGui::DragFloat("origin y", &origin.y))
			{
				sprite.setOrigin(origin);
			}
		}

		else if (circle_obj != nullptr)
		{
			auto& circle = circle_obj->GetCircle();

			const sf::Texture* attached_texture = circle.getTexture();

			ImGui::Text("texture");
			ImGui::SameLine();

			if (attached_texture != nullptr)
			{
				if (ImGui::ImageButton(*attached_texture, { 25, 25 }))
				{
					// TODO:
				}
			}
			else
			{
				if (ImGui::Button("##notex", { 25, 25 }))
				{

				}
			}

			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("move file");
				if (payload != nullptr)
				{
					std::filesystem::path src = *(std::string*)payload->Data;
					do
					{
						if (!src.has_extension() || (src.extension().string() != ".jpg" && src.extension().string() != ".png"))
						{
							break;
						}

						std::filesystem::path relative = std::filesystem::relative(src, asset_browser.GetAssetPath());
						if (ResourceManager::GetTextures().GetData().contains(relative.string()))
						{
							sf::Texture* managed_texture = ResourceManager::GetTextures().Get(relative.string());
							circle_obj->SetTexture(relative, managed_texture);
						}
						else
						{
							sf::Texture new_texture;
							if (!new_texture.loadFromFile(src.string()))
							{
								LOGERRORF("Failed to load texture from {}", src);
							}
							sf::Texture* managed_texture = ResourceManager::GetTextures().Add(relative.string(), new_texture);
							if (managed_texture != nullptr)
							{
								circle_obj->SetTexture(relative, managed_texture);
							}
						}

					} while (false);
				}
				ImGui::EndDragDropTarget();
			}

			if (attached_texture != nullptr)
			{
				if (ImGui::TreeNode("texture rect"))
				{
					sf::IntRect rect = circle.getTextureRect();
					if (ImGui::DragInt("left", &rect.left))
					{
						circle.setTextureRect(rect);
					}
					if (ImGui::DragInt("top", &rect.top))
					{
						circle.setTextureRect(rect);
					}
					if (ImGui::DragInt("width", &rect.width))
					{
						circle.setTextureRect(rect);
					}
					if (ImGui::DragInt("height", &rect.height))
					{
						circle.setTextureRect(rect);
					}

					ImGui::TreePop();
				}
			}

			const auto& circle_col = circle.getFillColor();
			float col[4] = {
				circle_col.r / 255.f,
				circle_col.g / 255.f,
				circle_col.b / 255.f,
				circle_col.a / 255.f
			};

			if (ImGui::ColorEdit4("fill color", col))
				circle.setFillColor(sf::Color{ 
				(uint8_t)(col[0] * 255.f), 
				(uint8_t)(col[1] * 255.f), 
				(uint8_t)(col[2] * 255.f), 
				(uint8_t)(col[3] * 255.f) });

			auto circle_radius = circle.getRadius();
			if (ImGui::DragFloat("radius", &circle_radius))
				circle.setRadius(circle_radius);

			Vec2f scale = circle.getScale();
			if (ImGui::DragFloat("scale x", &scale.x))
			{
				circle.setScale(scale);
			}
			if (ImGui::DragFloat("scale y", &scale.y))
			{
				circle.setScale(scale);
			}
		}

		else if (audio_obj != nullptr)
		{
			float volume = audio_obj->GetVolume();
			float pitch = audio_obj->GetPitch();
			Vec3f spatial_pos = audio_obj->GetAudioPosition();
			const AudioSource* audio_source = audio_obj->GetAudioSource();

			if (ImGui::SliderFloat("volume", &volume, 0, 100))
			{
				audio_obj->SetVolume(volume);
			}
			if (ImGui::SliderFloat("pitch", &pitch, 0, 2))
			{
				audio_obj->SetPitch(pitch);
			}

			ImGui::BeginDisabled(audio_obj->GetChannels() > 1 || audio_obj->GetChannels() == 0);
			if (
				ImGui::DragFloat("spatial x", &spatial_pos.x) ||
				ImGui::DragFloat("spatial y", &spatial_pos.y) ||
				ImGui::DragFloat("spatial z", &spatial_pos.z))
			{
				audio_obj->SetAudioPosition(spatial_pos);
			}
			ImGui::EndDisabled();

			ImGui::Text("source file");
			ImGui::SameLine();

			if (audio_source != nullptr && !audio_source->full_path.empty())
			{
				if (ImGui::Button(audio_obj->GetAudioSource()->relative_path.string().c_str(), { 25, 25 }))
				{
					// TODO:
				}
			}
			else
			{
				if (ImGui::Button("##notex", { 25, 25 }))
				{

				}
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("move file"))
				{
					std::filesystem::path src = *(std::string*)payload->Data;
					do
					{
						if (!src.has_extension() || (src.extension() != ".mp3" && src.extension() != ".wav" && src.extension() != ".ogg"))
						{
							break;
						}

						std::filesystem::path relative = std::filesystem::relative(src, asset_browser.GetAssetPath());
						AudioSource* managed_sound_buffer = ResourceManager::GetAudioSources().Get(relative.string());

						if (managed_sound_buffer != nullptr)
						{
							audio_obj->SetSource(managed_sound_buffer);
						}
						else
						{
							AudioSource new_src;
							new_src.full_path = src;
							new_src.relative_path = relative;

							if (!audio_obj->PlaysFromSource())
							{
								sf::SoundBuffer sound_buffer;
								if (!sound_buffer.loadFromFile(src.string()))
								{
									LOGERRORF("Failed to soundbuffer from {}", src);
								}

								new_src.sound_buffer = sound_buffer;
								new_src.has_valid_buffer = true;
							}
							else
							{
								new_src.has_valid_buffer = false;
							}

							AudioSource* managed_audio_source = ResourceManager::GetAudioSources().Add(relative.string(), new_src);
							if (managed_audio_source != nullptr)
							{
								audio_obj->SetSource(managed_audio_source);
							}
						}

					} while (false);
				}

				ImGui::EndDragDropTarget();
			}

			if (audio_source != nullptr && !audio_source->full_path.empty())
			{
				bool play_from_src = !audio_obj->PlaysFromSource();
				if (ImGui::Checkbox("cache source", &play_from_src))
				{
					audio_obj->ShouldPlayFromSource(!play_from_src);
				}

				if (ImGui::Button("Play"))
				{
					audio_obj->Play();
				}
				ImGui::SameLine();
				if (ImGui::Button("Pause"))
				{
					audio_obj->Pause();
				}
				ImGui::SameLine();
				if (ImGui::Button("Stop"))
				{
					audio_obj->Stop();
				}

				float time_line = audio_obj->GetTimeLine().asSeconds();
				if (ImGui::SliderFloat("time", &time_line, 0, audio_obj->GetDuration().asSeconds()))
				{
					audio_obj->SetTimeLine(time_line);
				}
			}
		}

		else if (text_obj != nullptr)
		{
			char buf[512];
			strncpy(buf, text_obj->GetText().c_str(), text_obj->GetText().length() + 1);
			if (ImGui::InputTextMultiline("Text", buf, sizeof(buf), {}))
			{
				text_obj->SetText(buf);
			}

			if (ImGui::TreeNode("Style properties"))
			{
				sf::Text& text = text_obj->GetTextObj();
				Toad::TextStyle text_style = text_obj->GetStyle();

				if (!text.getFont()->getInfo().family.empty())
				{
					if (ImGui::Button(text.getFont()->getInfo().family.c_str()))
					{
						// TODO:
					}
				}
				else
				{
					if (ImGui::Button("##nofont"))
					{

					}
				}

				if (ImGui::Button("set default font"))
				{
					sf::Font* font = ResourceManager::GetFonts().Get("Default");

					if (font == nullptr)
					{
						if (!font->loadFromFile("C:\\Windows\\Fonts\\Arial.ttf"))
						{
							LOGWARN("Can't find C:\\Windows\\Fonts\\Arial.ttf");
						}
						else
						{
							ResourceManager::GetFonts().Add("Default", *font);
							text_obj->SetFont("Default", *font);
						}
					}
					else
					{
						text_obj->SetFont("Default", *font);
					}
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("move file"))
					{
						std::filesystem::path src = *(std::string*)payload->Data;
						do
						{
							if (!src.has_extension() || (
								src.extension().string() != ".ttf" &&
								src.extension().string() != ".pfa" &&
								src.extension().string() != ".pfb" &&
								src.extension().string() != ".cff" &&
								src.extension().string() != ".otf" &&
								src.extension().string() != ".sfnt" &&
								src.extension().string() != ".pcf" &&
								src.extension().string() != ".fnt" &&
								src.extension().string() != ".bdf" &&
								src.extension().string() != ".pfr" &&
								src.extension().string() != ".t42"
								))
							{
								break;
							}

							std::filesystem::path relative = std::filesystem::relative(src, asset_browser.GetAssetPath());
							if (ResourceManager::GetFonts().GetData().contains(relative.string()))
							{
								sf::Font* managed_font = ResourceManager::GetFonts().Get(relative.string());
								text_obj->SetFont(relative, *managed_font);
							}
							else
							{
								sf::Font new_font;
								if (!new_font.loadFromFile(src.string()))
								{
									LOGERRORF("Failed to load font from {}", src);
								}
								sf::Font* managed_font = ResourceManager::GetFonts().Add(relative.string(), new_font);
								if (managed_font != nullptr)
								{
									text_obj->SetFont(relative, *managed_font);
								}
							}

						} while (false);
					}

					ImGui::EndDragDropTarget();
				}

				float fill_col[4] = {
					text_style.fill_col.r / 255.f,
					text_style.fill_col.g / 255.f,
					text_style.fill_col.b / 255.f,
					text_style.fill_col.a / 255.f
				};

				float outline_col[4] = {
					text_style.outline_col.r / 255.f,
					text_style.outline_col.g / 255.f,
					text_style.outline_col.b / 255.f,
					text_style.outline_col.a / 255.f
				};

				if (ImGui::ColorEdit4("fill color", fill_col))
				{
					sf::Color col = sf::Color(
						static_cast<uint8_t>(fill_col[0] * 255.f),
						static_cast<uint8_t>(fill_col[1] * 255.f),
						static_cast<uint8_t>(fill_col[2] * 255.f),
						static_cast<uint8_t>(fill_col[3] * 255.f)
					);

					text.setFillColor(col);

					text_style.fill_col = col;
					text_obj->SetStyle(text_style, false);
				}

				if (ImGui::ColorEdit4("outline color", outline_col))
				{
					sf::Color col = sf::Color(
						static_cast<uint8_t>(outline_col[0] * 255.f),
						static_cast<uint8_t>(outline_col[1] * 255.f),
						static_cast<uint8_t>(outline_col[2] * 255.f),
						static_cast<uint8_t>(outline_col[3] * 255.f)
					);

					text.setOutlineColor(col);

					text_style.outline_col = col;
					text_obj->SetStyle(text_style, false);
				}
				int char_size_int = (int)text_style.char_size;
				if (ImGui::InputInt("character size", &char_size_int, 1, 3))
				{
					char_size_int = std::clamp(char_size_int, 0, INT_MAX);
					text_style.char_size = char_size_int;
					text.setCharacterSize(text_style.char_size);
					text_obj->SetStyle(text_style, false);
				}
				if (ImGui::InputFloat("character spacing", &text_style.char_spacing, 1, 3))
				{
					text.setLetterSpacing(text_style.char_spacing);
					text_obj->SetStyle(text_style, false);
				}
				if (ImGui::InputFloat("line spacing", &text_style.line_spacing, 1, 3))
				{
					text.setLineSpacing(text_style.line_spacing);
					text_obj->SetStyle(text_style, false);
				}
				if (ImGui::InputFloat("outline thickness", &text_style.outline_thickness, 1, 3))
				{
					text.setOutlineThickness(text_style.outline_thickness);
					text_obj->SetStyle(text_style, false);
				}

				if (ImGui::BeginCombo("style", ""))
				{
					if (ImGui::Selectable("Regular", text_style.style & sf::Text::Style::Regular))
					{
						text_style.style = static_cast<sf::Text::Style>(static_cast<int>(text_style.style) ^ static_cast<int>(sf::Text::Style::Regular));
						text_obj->SetStyle(text_style);
					}
					if (ImGui::Selectable("Bold", text_style.style & sf::Text::Style::Bold))
					{
						text_style.style = static_cast<sf::Text::Style>(static_cast<int>(text_style.style) ^ static_cast<int>(sf::Text::Style::Bold));
						text_obj->SetStyle(text_style);
					}
					if (ImGui::Selectable("Italic", text_style.style & sf::Text::Style::Italic))
					{
						text_style.style = static_cast<sf::Text::Style>(static_cast<int>(text_style.style) ^ static_cast<int>(sf::Text::Style::Italic));
						text_obj->SetStyle(text_style);
					}
					if (ImGui::Selectable("Underlined", text_style.style & sf::Text::Style::Underlined))
					{
						text_style.style = static_cast<sf::Text::Style>(static_cast<int>(text_style.style) ^ static_cast<int>(sf::Text::Style::Underlined));
						text_obj->SetStyle(text_style);
					}
					if (ImGui::Selectable("StrikeThrough", text_style.style & sf::Text::Style::StrikeThrough))
					{
						text_style.style = static_cast<sf::Text::Style>(static_cast<int>(text_style.style) ^ static_cast<int>(sf::Text::Style::StrikeThrough));
						text_obj->SetStyle(text_style);
					}

					ImGui::EndCombo();
				}


				ImGui::TreePop();
			}
		}

		else if (cam_obj != nullptr)
		{
			bool active = cam_obj->IsActive();
			if (ImGui::Checkbox("Active", &active))
			{
				if (active)
				{
					cam_obj->ActivateCamera();
				}
				else
				{
					cam_obj->DeactivateCamera();
				}
			}

			Vec2f size = cam_obj->GetSize();
			if (ImGui::DragFloat("size x", &size.x))
			{
				cam_obj->SetSize(size);
			}

			if (ImGui::DragFloat("size y", &size.y))
			{
				cam_obj->SetSize(size);
			}
		}

		static std::string selected_script_name;

		bool script_node_open = ImGui::TreeNode("Scripts");
		if (ImGui::IsMouseReleased(1) && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
		{
			ImGui::OpenPopup("SCRIPT ADD");
		}
		if (ImGui::BeginPopup("SCRIPT ADD"))
		{
			for (auto& [name, script] : Toad::Engine::Get().GetGameScriptsRegister())
			{
				if (!script)
					continue;

				if (ImGui::Button(name.c_str()))
				{
					selected_obj->AddScript(script->Clone());
					selected_obj->GetScript(name)->ExposeVars();
				}
			}

			ImGui::EndPopup();
		}

		if (script_node_open)
		{
			// show attached scripts
			for (auto& [name, script] : attached_scripts)
			{
				if (!script)
				{
					ImGui::TextColored({ 1,1,0,1 }, "Script %s is null", name.c_str());
					continue;
				}

				// script properties
				if (ImGui::TreeNode(("SCRIPT " + name).c_str()))
				{
					ImGui::SameLine();
					ImGui::TextColored({ 0.2f,0.2f,0.2f,1.f }, "%p", script);

					auto script_vars = script->GetReflection().Get();
					const char* no_var_found_msg = "This variable is not found/exposed in this script, Resave the scene or re-expose the variable";
					for (auto& [name, var] : script_vars.str)
					{
						if (var)
						{
							char buf[100];
							strncpy(buf, var->c_str(), sizeof buf);
							if (ImGui::InputText(name.c_str(), buf, sizeof buf))
							{
								*var = buf;
							}
							ImGui::SameLine();
							ImGui::TextColored({ 0.2f,0.2f,0.2f,1.f }, "%p", var);
						}
						else
						{
							ImGui::TextColored({ 1.f,0.f,0.f,1.f }, "%s is null", name.c_str());
							ImGui::SameLine();
							HelpMarker(no_var_found_msg);
						}
					}
					for (auto& [name, var] : script_vars.b)
					{
						if (var)
						{
							ImGui::Checkbox(name.c_str(), var);
							ImGui::SameLine();
							ImGui::TextColored({ 0.2f,0.2f,0.2f,1.f }, "%p", var);
						}
						else
						{
							ImGui::TextColored({ 1.f,0.f,0.f,1.f }, "%s is null", name.c_str());
							ImGui::SameLine();
							HelpMarker(no_var_found_msg);
						}
					}
					for (auto& [name, var] : script_vars.flt)
					{
						if (var)
						{
							ImGui::DragFloat(name.c_str(), var);
							ImGui::SameLine();
							ImGui::TextColored({ 0.2f,0.2f,0.2f,1.f }, "%p", var);
						}
						else
						{
							ImGui::TextColored({ 1.f,0.f,0.f,1.f }, "%s is null", name.c_str());
							ImGui::SameLine();
							HelpMarker(no_var_found_msg);
						}

					}
					for (auto& [name, var] : script_vars.i8)
					{
						if (var)
						{
							ImGui::DragInt(name.c_str(), (int*)var, 1.0f, i8_min, i8_max);
							ImGui::SameLine();
							ImGui::TextColored({ 0.2f,0.2f,0.2f,1.f }, "%p", var);
						}
						else
						{
							ImGui::TextColored({ 1.f,0.f,0.f,1.f }, "%s is null", name.c_str());
							ImGui::SameLine();
							HelpMarker(no_var_found_msg);
						}
					}
					for (auto& [name, var] : script_vars.i16)
					{
						if (var)
						{
							ImGui::DragInt(name.c_str(), (int*)var, 1.0f, i16_min, i16_max);
							ImGui::SameLine();
							ImGui::TextColored({ 0.2f,0.2f,0.2f,1.f }, "%p", var);
						}
						else
						{
							ImGui::TextColored({ 1.f,0.f,0.f,1.f }, "%s is null", name.c_str());
							ImGui::SameLine();
							HelpMarker(no_var_found_msg);
						}
					}
					for (auto& [name, var] : script_vars.i32)
					{
						if (var)
						{
							ImGui::DragInt(name.c_str(), var);
							ImGui::SameLine();
							ImGui::TextColored({ 0.2f,0.2f,0.2f,1.f }, "%p", var);
						}
						else
						{
							ImGui::TextColored({ 1.f,0.f,0.f,1.f }, "%s is null", name.c_str());
							ImGui::SameLine();
							HelpMarker(no_var_found_msg);
						}
					}

					ImGui::SeparatorText("OnEditorUI");

					static bool once = false;

					script->OnEditorUI(selected_obj, ImGui::GetCurrentContext());

					ImGui::TreePop();
				}

				// right clicking on script causes popup with extra options
				if (ImGui::IsMouseReleased(1) && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
				{
					selected_script_name = name;
					ImGui::OpenPopup("SCRIPT SETTINGS");
				}
			}

			if (ImGui::BeginPopup("SCRIPT SETTINGS"))
			{
				// show options for this script
				if (ImGui::Button("SCRIPT DELETE"))
				{
					selected_obj->RemoveScript(selected_script_name);
				}
				if (ImGui::Button("SCRIPT COPY"))
				{
					// selected_obj->SerializeScripts()
				}
				ImGui::EndPopup();
			}

			ImGui::TreePop();
		}
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
