#include "pch.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "Engine/Engine.h"
#include "ui.h"

#include "imgui-SFML.h"
#include "FileBrowser.h"
#include "GameAssetsBrowser.h"
#include "TextEditor.h"

#include "engine/systems/build/package.h"
#include "project/ToadProject.h"

using json = nlohmann::json;

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
	static bool project_load_popup_select = false;

	static project::ProjectSettings settings{};
	static Toad::FileBrowser fBrowser(std::filesystem::current_path().string());
	static Toad::GameAssetsBrowser asset_browser(settings.project_path);
	static Toad::TextEditor textEditor;

	static std::shared_ptr<Toad::Object> selected_obj = nullptr;
	static std::shared_ptr<Toad::Sprite> selected_sprite = nullptr;

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
			if (ImGui::MenuItem("Reload"))
			{
				Toad::Engine::Get().LoadGameScripts();
			}
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

					Toad::SaveScene(scene, fBrowser.GetPath());
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
						asset_browser.SetAssetPath((std::filesystem::path(settings.project_path) / settings.name / (settings.name + "_GAME") / "src" / "assets").string());
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
			Toad::SaveScene(Toad::Engine::Get().GetScene(), asset_browser.GetAssetPath());
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

		for (auto& [name, obj] : Toad::Engine::Get().GetScene().objects_map)
		{
			index++;

			if (ImGui::Selectable(name.c_str(), selected_objects.contains(name) || (selected_obj != nullptr && selected_obj->name == name)))
			{
				if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && selected_obj != nullptr && selected_obj->name != name)
				{
					if (selected_objects.contains(name))
					{
						selected_objects.erase(name);
					}
					else
					{
						selected_objects.insert(name);
					}
				}
				else if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && selected_obj != nullptr && selected_obj->name != name)
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
				if (!ImGui::IsPopupOpen("SceneModifyPopup"))
				{
					ImGui::OpenPopup("SceneModifyPopup");
					ignore_mouse_click = true;
				}
			}
		}

		if (check_range)
		{
			std::vector<std::string> keys;
			int origin = 0;
			int j = 0;
			for (const std::string& name : Toad::Engine::Get().GetScene().objects_map | std::views::keys)
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

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

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

		while (!remove_objects_queue.empty())
		{
			const std::string& front = remove_objects_queue.front();
			selected_objects.erase(front);
			Toad::Engine::Get().GetScene().RemoveFromScene(front);
			remove_objects_queue.pop();
		}

		ImGui::End();
	}


	ImGui::Begin("Inspector", nullptr);
	{
		if (selected_obj != nullptr)
		{
			auto attached_scripts = selected_obj->GetAttachedScripts();

			ImGui::Text(selected_obj->name.c_str());

			auto sprite_obj = dynamic_cast<Toad::Sprite*>(selected_obj.get());
			auto circle_obj = dynamic_cast<Toad::Circle*>(selected_obj.get());

			// show sprite properties
			if (sprite_obj != nullptr)
			{
				auto& sprite = sprite_obj->GetSprite();

				const sf::Texture* attached_texture = sprite.getTexture();
				auto pos = sprite.getPosition();

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
							if (!src.has_extension() && (src.extension().string() != ".jpg" || src.extension().string() != ".png")) 
							{
								break;
							}
						
							std::filesystem::path relative = std::filesystem::relative(src, asset_browser.GetAssetPath());
							Toad::ResourceManager& resource_manager = Toad::Engine::Get().GetResourceManager();
							if (resource_manager.GetTextures().contains(relative.string()))
							{
								sf::Texture* managed_texture = resource_manager.GetTexture(relative.string());
								sprite_obj->SetTexture(relative, *managed_texture);
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
									sprite_obj->SetTexture(relative, *managed_texture);
								}
							}

						} while (false);
					}

					ImGui::EndDragDropTarget();
				}

				if (ImGui::DragFloat("X", &pos.x))
					sprite.setPosition(pos);
				if (ImGui::DragFloat("Y", &pos.y))
					sprite.setPosition(pos);

				auto colorU32 = ImGui::ColorConvertU32ToFloat4(sprite.getColor().toInteger());
				float col[4] = { colorU32.x, colorU32.y, colorU32.z, colorU32.w };
				if (ImGui::ColorEdit4("color", col))
					sprite.setColor(sf::Color(ImGui::ColorConvertFloat4ToU32({ col[0], col[1], col[2], col[3] })));
			}

			// show circle properties
			else if (circle_obj != nullptr)
			{
				auto& circle = circle_obj->GetCircle();

				auto pos = circle.getPosition();

				if (ImGui::DragFloat("X", &pos.x))
					circle.setPosition(pos);
				if (ImGui::DragFloat("Y", &pos.y))
					circle.setPosition(pos);

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

						constexpr int i8_min = std::numeric_limits<int8_t>::min();
						constexpr int i8_max = std::numeric_limits<int8_t>::max();
						constexpr int i16_min = std::numeric_limits<int16_t>::min();
						constexpr int i16_max = std::numeric_limits<int16_t>::max();

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
				Toad::Engine::Get().SetScene(Toad::LoadScene(file));
			}
		}
	}

    ImGui::End();

	ImGui::Begin("Game Assets");

	asset_browser.Show();

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
