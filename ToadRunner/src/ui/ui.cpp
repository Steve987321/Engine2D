#include "pch.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "Engine/Engine.h"

#include "EngineMeta.h"

#include "ui.h"

#include "imgui-SFML.h"
#include "FileBrowser.h"
#include "GameAssetsBrowser.h"
#include "TextEditor.h"

#include "engine/systems/build/package.h"
#include "project/ToadProject.h"

using json = nlohmann::json;

constexpr int i8_min = std::numeric_limits<int8_t>::min();
constexpr int i8_max = std::numeric_limits<int8_t>::max();
constexpr int i16_min = std::numeric_limits<int16_t>::min();
constexpr int i16_max = std::numeric_limits<int16_t>::max();
constexpr int i32_max = std::numeric_limits<int32_t>::max();

void ui::engine_ui(ImGuiContext* ctx)
{
	ImGui::SetCurrentContext(ctx);

	constexpr auto dock_window_flags =
		ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiButtonFlags_NoNavFocus;

	ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos);

	ImGuiID project_creation_popup_id = ImHashStr("CreateProject");
	ImGuiID project_load_popup_id = ImHashStr("LoadProject");
	ImGuiID project_package_popup_id = ImHashStr("PackageProject");
	ImGuiID save_scene_popup_id = ImHashStr("SaveScene");
	ImGuiID scene_modify_popup_id = ImHashStr("SceneModifyPopup");
	static bool project_load_popup_select = false;

	static project::ProjectSettings settings{};
	static Toad::FileBrowser fBrowser(std::filesystem::current_path().string());
	static Toad::GameAssetsBrowser asset_browser(settings.project_path);
	static Toad::TextEditor textEditor;

	static Toad::Object* selected_obj = nullptr;

	static std::string clipboard_data;


	//LOGDEBUGF("{} {}", ImGui::GetMousePos().x, ImGui::GetMousePos().y);

	ImGui::Begin("DockSpace", nullptr, dock_window_flags);
	ImGui::DockSpace(ImGui::GetID("DockSpace"));
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Engine"))
		{
			if (ImGui::MenuItem("Create Project"))
			{
				settings.engine_path = (std::filesystem::current_path().parent_path() / "Engine").string();

				ImGui::PushOverrideID(project_creation_popup_id);
				ImGui::OpenPopup("CreateProject");
				ImGui::PopID();

			}
			if (ImGui::MenuItem("Load Project"))
			{
				project_load_popup_select = true;

				ImGui::PushOverrideID(project_load_popup_id);
				ImGui::OpenPopup("LoadProject");
				ImGui::PopID();
			}
			ImGui::BeginDisabled(project::current_project.name.empty());
			if (ImGui::MenuItem("Package"))
			{
				ImGui::PushOverrideID(project_package_popup_id);
				ImGui::OpenPopup("PackageProject");
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
				Toad::Engine::Get().LoadGameScripts();
			}
			ImGui::EndDisabled();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Scene"))
		{
			auto& scene = Toad::Engine::Get().GetScene();

			if (scene.objects_map.empty())
			{
				ImGui::BeginDisabled();
			}
			if (ImGui::MenuItem("Save"))
			{
				if (!scene.objects_map.empty())
				{
					ImGui::PushOverrideID(save_scene_popup_id);
					ImGui::OpenPopup("SaveScene");
					ImGui::PopID();
				}
			}
			if (scene.objects_map.empty())
			{
				ImGui::EndDisabled();
			}

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

		if (created_project)
		{
			if (cpri.res != project::CREATE_PROJECT_RES::OK)
			{
				if (ImGui::Button("Close"))
				{
					created_project = false;
				}

				ImGui::TextColored({ 1,0,0,1 }, Toad::format_str("Project creation failed: {}, {}", cpri.res, cpri.description).c_str());
			}
			else
			{
				if (ImGui::Button("Close"))
				{
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

			ImGui::Text("Selected Path %s", settings.project_path.empty() ? "?" : settings.project_path.c_str());

			if (ImGui::Button("Select Project Directory"))
			{
				auto selected = Toad::GetPathDialog("Select project directory", settings.engine_path);
				settings.project_path = selected;
			}

			if (!settings.name.empty() && !settings.project_path.empty() && !settings.engine_path.empty())
			{
				if (ImGui::Button("Create"))
				{
					created_project = true;

					cpri = Create(settings);

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

					Toad::Engine::Get().UpdateGameBinPaths(settings.name + ".dll", settings.project_path + "bin\\Dev-windows-x86_64");
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
					Toad::Engine::Get().UpdateGameBinPaths(project::current_project.name + "_Game.dll", project::current_project.project_path + "\\bin\\Dev-windows-x86_64");

					Toad::Engine::Get().LoadGameScripts();

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

		ImGui::Text("selected path: %s", output_path.c_str());
		if (ImGui::Button("Select output directory"))
		{			
			output_path = Toad::GetPathDialog("select output directory", std::filesystem::current_path().string());
		}

		ImGui::BeginDisabled(output_path.empty());
		if (ImGui::Button("Create"))
		{
			static Toad::Package package;
			std::filesystem::path proj_file;
			for (const auto& entry : std::filesystem::directory_iterator(project::current_project.project_path))
			{
				if (entry.path().has_extension() && entry.path().extension() == ".TOADPROJECT")
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

			package.CreatePackage(proj_file, output_path, misc::current_editor.path);
		}
		ImGui::EndDisabled();

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
			Toad::Engine::Get().GetScene().name = scene_name;
			SaveScene(Toad::Engine::Get().GetScene(), asset_browser.GetAssetPath());
		}

		ImGui::EndPopup();
	}
	ImGui::PopID();

	ImGui::Begin("Settings", nullptr);
	{
		ImGui::Text("FPS %.1f", 1.f / Toad::Engine::Get().GetDeltaTime().asSeconds());

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
			for (const auto& [name, obj] : Toad::Engine::Get().GetScene().objects_map)
			{
				ImGui::Text("name %s %p", name.c_str(), obj.get());
				for (const auto& [namescript, script] : obj->GetAttachedScripts())
				{
					ImGui::Text("script %s %p", namescript.c_str(), script.get());
					for (const auto& [varname, pvar] : script->GetReflection().vars.b)
					{
						ImGui::Text("bool %s %p", varname.c_str(), pvar);
					}
				}
			}

			ImGui::TreePop();
		}
		ImGui::End();
	}

	// SCENE/HIERARCHY
	ImGui::Begin("Scene", nullptr);
	{
		ImGui::SeparatorText(Toad::Engine::Get().GetScene().name.c_str());
		bool ignore_mouse_click = false;
		std::queue<std::string> remove_objects_queue;

		if (ImGui::IsWindowFocused())
		{
			if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
			{
				if (ImGui::IsKeyPressed(ImGuiKey_V))
				{
					if (!clipboard_data.empty())
					{

					}
				}
				else if (ImGui::IsKeyPressed(ImGuiKey_C))
				{
					clipboard_data;
				}
			}
		}

		static std::set<std::string> selected_objects = {};
		int index = 0;
		static size_t prev_cursor_index = 0;
		static size_t cursor_index = 0;
		static bool cursor_index_is_under = false;
		static bool check_range = false;

		std::vector<std::string> scene_objects {};
		scene_objects.reserve(Toad::Engine::Get().GetScene().objects_map.size());

		for (const auto& [obj_name, obj] : Toad::Engine::Get().GetScene().objects_map)
		{
			if (!obj->GetParent().empty())
			{
				continue;
			}

			scene_objects.emplace_back(obj_name);

			for (const std::string& name : obj->GetChildren())
			{
				scene_objects.emplace_back(name);
			}
		}

		const std::function<void(Toad::Object*) > recursive_iterate_children = [&](Toad::Object* obj)
			{
				index++;

				if (obj->GetChildren().empty())
				{
					if (ImGui::Selectable(obj->name.c_str(), selected_objects.contains(obj->name) || (selected_obj != nullptr && selected_obj->name == obj->name)))
					{
						if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && selected_obj != nullptr && selected_obj->name != obj->name)
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
								if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && selected_obj != nullptr && selected_obj->name != obj->name)
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

						for (Toad::Object* child : obj->GetChildrenAsObjects())
						{
							recursive_iterate_children(child);
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
									if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && selected_obj != nullptr && selected_obj->name != child->name)
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
										selected_obj = obj;
										selected_objects.clear();
									}
								}
							}
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
								if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && selected_obj != nullptr && selected_obj->name != obj->name)
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
			};
		for (auto& name : scene_objects)
		{
			Toad::Object* obj = Toad::Engine::Get().GetScene().GetSceneObject(name);
			// make sure we only iterate over root objects
			if (!obj->GetParent().empty())
			{
				continue;
			}

			recursive_iterate_children(obj);
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
					if (!cursor_index_is_under)
					{
						origin -= 1;
					}
				}
				keys.push_back(name);
			}

			//LOGDEBUGF("prev_index={} is_under={} cursor_index={} origin={}", prev_cursor_index, cursor_index_is_under, cursor_index, origin);
			
			selected_objects.clear();
			for (int i = origin; cursor_index_is_under ? i < static_cast<int>(cursor_index) : i > static_cast<int>(cursor_index) - 2; cursor_index_is_under ? i++ : i--)
			{
				selected_objects.insert(keys[i]);
			}

			check_range = false;
		}

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()))
		{
			selected_obj = nullptr;
			selected_objects.clear();
		}

		if (!ignore_mouse_click && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			if (ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()))
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
				if (ImGui::MenuItem("Circle"))
				{
					Toad::Engine::Get().GetScene().AddToScene(Toad::Circle("Circle"));
				}
				if (ImGui::MenuItem("Sprite"))
				{
					Toad::Engine::Get().GetScene().AddToScene(Toad::Sprite("Sprite"));
				}
				if (ImGui::MenuItem("Audio"))
				{
					Toad::Engine::Get().GetScene().AddToScene(Toad::Audio("Audio"));
				}
				if (ImGui::MenuItem("Text"))
				{
					Toad::Engine::Get().GetScene().AddToScene(Toad::Text("Text"));
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
			Toad::Object* obj = Toad::Engine::Get().GetScene().GetSceneObject(front);
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
		if (selected_obj != nullptr)
		{
			auto attached_scripts = selected_obj->GetAttachedScripts();
			bool suggestion = false;
			char name_buf[100];
			std::string new_name_str;
			strcpy_s(name_buf, selected_obj->name.c_str());

			ImVec2 input_name_pos = ImGui::GetCursorPos();
			if (ImGui::InputText("name", name_buf, sizeof(name_buf)))
			{
				Toad::Scene& scene = Toad::Engine::Get().GetScene();

				new_name_str = name_buf;
				
				if (new_name_str != selected_obj->name && scene.objects_map.contains(new_name_str))
				{
					suggestion = true;

					auto count = scene.objects_map.count(new_name_str);
					new_name_str += " (" + std::to_string(count) + ')';
					while (scene.objects_map.contains(new_name_str))
					{
						new_name_str += " (" + std::to_string(++count) + ')';
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
					auto& map = Toad::Engine::Get().GetScene().objects_map;
					if (!map.contains(new_name_str))
					{
						auto a = map.extract(selected_obj->name);
						a.key() = new_name_str;
						map.insert(std::move(a));

						selected_obj->name = new_name_str;
					}
				}
			}

			if (suggestion)
			{
				//ImGuiInputTextState* state = ImGui::GetInputTextState(ImGui::GetID("name"));
				std::string part_of_str = new_name_str.substr(strlen(name_buf), new_name_str.length() - strlen(name_buf));
				ImVec2 frame_pad = ImGui::GetStyle().FramePadding;
				ImVec2 text_size = ImGui::CalcTextSize(name_buf);
				ImGui::SetCursorPos({ input_name_pos.x + text_size.x + frame_pad.x, input_name_pos.y + frame_pad.y });
				ImGui::TextColored({ 1,1,1,0.4f }, part_of_str.c_str());
			}
			
			ImGui::Text(selected_obj->name.c_str());

			float pos_x = selected_obj->GetPosition().x;
			float pos_y = selected_obj->GetPosition().y;
			if (ImGui::DragFloat("x", &pos_x))
			{
				selected_obj->SetPosition({ pos_x, pos_y });
			}
			if (ImGui::DragFloat("y", &pos_y))
			{
				selected_obj->SetPosition({ pos_x, pos_y });
			}

			//if (ImGui::Button("Test"))
			//{
			//	auto child_obj = Toad::Engine::Get().GetScene().AddToScene(Toad::Circle("child object"));
			//	child_obj->SetParent(selected_obj);
			//}

			auto sprite_obj = dynamic_cast<Toad::Sprite*>(selected_obj);
			auto circle_obj = dynamic_cast<Toad::Circle*>(selected_obj);
			auto audio_obj = dynamic_cast<Toad::Audio*>(selected_obj);
			auto text_obj = dynamic_cast<Toad::Text*>(selected_obj);

			if (sprite_obj != nullptr)
			{
				auto& sprite = sprite_obj->GetSprite();

				const sf::Texture* attached_texture = sprite.getTexture();

				ImGui::Text("texture");
				ImGui::SameLine();

				if (attached_texture != nullptr)
				{
					if (ImGui::ImageButton(*attached_texture, {25, 25}))
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
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("move file"))
					{
						std::filesystem::path src = *(std::string*)payload->Data;
						do
						{
							if (!src.has_extension() || (src.extension().string() != ".jpg" && src.extension().string() != ".png")) 
							{
								break;
							}
						
							std::filesystem::path relative = std::filesystem::relative(src, asset_browser.GetAssetPath());
							Toad::ResourceManager& resource_manager = Toad::Engine::Get().GetResourceManager();
							if (resource_manager.GetTextures().contains(relative.string()))
							{
								sf::Texture* managed_texture = resource_manager.GetTexture(relative.string());
								sprite_obj->SetTexture(relative, managed_texture);
							}
							else
							{
								sf::Texture new_texture;
								if (!new_texture.loadFromFile(src.string()))
								{
									LOGERRORF("Failed to load texture from {}", src);
								}
								sf::Texture* managed_texture = resource_manager.AddTexture(relative.string(), new_texture);
								if (managed_texture != nullptr)
								{
									sprite_obj->SetTexture(relative, managed_texture);
								}
							}

						} while (false);
					}

					ImGui::EndDragDropTarget();
				}

				/*if (ImGui::DragFloat("X", &pos.x))
					sprite.setPosition(pos);
				if (ImGui::DragFloat("Y", &pos.y))
					sprite.setPosition(pos);*/

				auto colorU32 = ImGui::ColorConvertU32ToFloat4(sprite.getColor().toInteger());
				float col[4] = { colorU32.x, colorU32.y, colorU32.z, colorU32.w };
				if (ImGui::ColorEdit4("color", col))
					sprite.setColor(sf::Color(ImGui::ColorConvertFloat4ToU32({ col[0], col[1], col[2], col[3] })));
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
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("move file"))
					{
						std::filesystem::path src = *(std::string*)payload->Data;
						do
						{
							if (!src.has_extension() || (src.extension().string() != ".jpg" && src.extension().string() != ".png"))
							{
								break;
							}

							std::filesystem::path relative = std::filesystem::relative(src, asset_browser.GetAssetPath());
							Toad::ResourceManager& resource_manager = Toad::Engine::Get().GetResourceManager();
							if (resource_manager.GetTextures().contains(relative.string()))
							{
								sf::Texture* managed_texture = resource_manager.GetTexture(relative.string());
								circle_obj->SetTexture(relative, managed_texture);
							}
							else
							{
								sf::Texture new_texture;
								if (!new_texture.loadFromFile(src.string()))
								{
									LOGERRORF("Failed to load texture from {}", src);
								}
								sf::Texture* managed_texture = resource_manager.AddTexture(relative.string(), new_texture);
								if (managed_texture != nullptr)
								{
									circle_obj->SetTexture(relative, managed_texture);
								}
							}

						} while (false);
					}

					ImGui::EndDragDropTarget();
				}

				const auto& circle_col = circle.getFillColor();
				float col[4] = {
					circle_col.r / 255.f,
					circle_col.g / 255.f,
					circle_col.b / 255.f,
					circle_col.a / 255.f
				};

				if (ImGui::ColorEdit4("fill color", col))
					circle.setFillColor(sf::Color(col[0] * 255, col[1] * 255, col[2] * 255, col[3] * 255));

				auto circle_radius = circle.getRadius();
				if (ImGui::DragFloat("radius", &circle_radius))
					circle.setRadius(circle_radius);
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
							Toad::ResourceManager& resource_manager = Toad::Engine::Get().GetResourceManager();
							AudioSource* managed_sound_buffer = resource_manager.GetAudioSource(relative.string());

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

								AudioSource* managed_audio_source = resource_manager.AddAudioSource(relative.string(), new_src);
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
				strcpy_s(buf, text_obj->GetText().c_str());
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
						Toad::ResourceManager& resource_manager = Toad::Engine::Get().GetResourceManager();
						sf::Font* font = resource_manager.GetFont("Default");

						if (font == nullptr)
						{
							if (!font->loadFromFile("C:\\Windows\\Fonts\\Arial.ttf"))
							{
								LOGWARN("Can't find C:\\Windows\\Fonts\\Arial.ttf");
							}
							else
							{
								resource_manager.AddFont("Default", *font);
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
								Toad::ResourceManager& resource_manager = Toad::Engine::Get().GetResourceManager();
								if (resource_manager.GetFonts().contains(relative.string()))
								{
									sf::Font* managed_font = resource_manager.GetFont(relative.string());
									text_obj->SetFont(relative, *managed_font);
								}
								else
								{
									sf::Font new_font;
									if (!new_font.loadFromFile(src.string()))
									{
										LOGERRORF("Failed to load font from {}", src);
									}
									sf::Font* managed_font = resource_manager.AddFont(relative.string(), new_font);
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
						ImGui::TextColored({ 0.2f,0.2f,0.2f,1.f }, "%p", script.get());

						auto script_vars = script->GetReflection().Get();

						for (auto& [name, var] : script_vars.str)
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
						for (auto& [name, var] : script_vars.b)
						{
							ImGui::Checkbox(name.c_str(), var);
							ImGui::SameLine();
							ImGui::TextColored({ 0.2f,0.2f,0.2f,1.f }, "%p", var);
						}
						for (auto& [name, var] : script_vars.flt)
						{
							ImGui::DragFloat(name.c_str(), var);
							ImGui::SameLine();
							ImGui::TextColored({ 0.2f,0.2f,0.2f,1.f }, "%p", var);
						}
						for (auto& [name, var] : script_vars.i8)
						{
							ImGui::DragInt(name.c_str(), (int*)var, 1.0f, i8_min, i8_max);
							ImGui::SameLine();
							ImGui::TextColored({ 0.2f,0.2f,0.2f,1.f }, "%p", var);
						}
						for (auto& [name, var] : script_vars.i16)
						{
							ImGui::DragInt(name.c_str(), (int*)var, 1.0f, i16_min, i16_max);
							ImGui::SameLine();
							ImGui::TextColored({ 0.2f,0.2f,0.2f,1.f }, "%p", var);
						}
						for (auto& [name, var] : script_vars.i32)
						{
							ImGui::DragInt(name.c_str(), var);
							ImGui::SameLine();
							ImGui::TextColored({ 0.2f,0.2f,0.2f,1.f }, "%p", var);
						}

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
					if (ImGui::Button("Remove"))
					{
						selected_obj->RemoveScript(selected_script_name);
					}

					ImGui::EndPopup();
				}

				ImGui::TreePop();
			}
		}

		ImGui::End();
	}

	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_HorizontalScrollbar);
	{
		auto& window_texture = Toad::Engine::Get().GetWindowTexture();

		auto content_size = ImGui::GetContentRegionAvail();

		// resize but keep aspect ratio
		constexpr float ar = 16.f / 9.f;
		float image_width = content_size.x;
		float image_height = content_size.x / ar;

		if (image_height > content_size.y)
		{
			image_height = content_size.y;
			image_width = content_size.y * ar;
		}

		// we want texture in the center 
		ImGui::SetCursorPos({
			(content_size.x - image_width) * 0.5f,
			(content_size.y - image_height) * 0.5f
		});

		ImGui::Image(window_texture, {image_width, image_height}, sf::Color::White);

		ImGui::SetCursorPos({ ImGui::GetScrollX() + 20, 20 });
		if (ImGui::TreeNode("Viewport Options"))
		{
			if (!Toad::Engine::Get().GameStateIsPlaying())
			{
				if (ImGui::Button("Play"))
					Toad::Engine::Get().StartGameSession();
			}
			else
			{
				if (ImGui::Button("Stop"))
					Toad::Engine::Get().StopGameSession();
			}

			ImGui::TreePop();
		}
		
		ImGui::End();
	}

    ImGui::Begin("File Browser", nullptr);
    fBrowser.Show();

	if (fBrowser.IsDoubleClicked())
	{
		auto file = std::filesystem::path(fBrowser.GetSelectedFile());
		if (file.has_extension())
		{
			auto ext = file.extension().string();

			if (ext == ".TSCENE")
			{
				Toad::Engine::Get().SetScene(Toad::LoadScene(file, asset_browser.GetAssetPath()));
				selected_obj = nullptr;
			}
		}
	}

    ImGui::End();

	ImGui::Begin("Game Assets");

	asset_browser.Show();
	if (asset_browser.loaded_scene)
	{
		selected_obj = nullptr;
	}
	ImGui::End();

    ImGui::Begin("Text Editor");

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
