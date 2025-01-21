#include "pch.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "SceneHierarchy.h"

#include "engine/Engine.h"

#include "UI.h"

using json = nlohmann::ordered_json;
using namespace Toad;

namespace ui
{
	void ShowSceneHierarchy()
	{
		// make sure this is only true on click
		hierarchy_clicked_object = false;

		ImGui::Begin("Scene", nullptr);
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
			Scene& scene = Scene::current_scene;

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
				std::vector<Object*> objects;
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
					LoadSceneObjects(data_json, scene, "asset_browser.GetAssetPath()", false);
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
		const std::function<void(Object*, bool) > recursive_iterate_children = [&](Object* obj, bool dontchangeskip)
			{
				index++;
				static bool skip = false;

				const auto drag_drop = [](bool& skip, Object* obj) {
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
								Object* as_object = Scene::current_scene.GetSceneObject(o).get();

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

						for (Object* child : obj->GetChildrenAsObjects())
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
				else if (child_obj->GetParent() == drag_drop_extra_check_parent)
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

		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			ImGui::SetWindowFocus("Inspector");
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
					Scene::current_scene.AddToScene(Object("Object"), Engine::Get().GameStateIsPlaying());

				if (ImGui::MenuItem("Circle"))
					Scene::current_scene.AddToScene(Circle("Circle"), Engine::Get().GameStateIsPlaying());

				if (ImGui::MenuItem("Sprite"))
					Scene::current_scene.AddToScene(Sprite("Sprite"), Engine::Get().GameStateIsPlaying());

				if (ImGui::MenuItem("Audio"))
					Scene::current_scene.AddToScene(Audio("Audio"), Engine::Get().GameStateIsPlaying());

				if (ImGui::MenuItem("Text"))
					Scene::current_scene.AddToScene(Text("Text"), Engine::Get().GameStateIsPlaying());

				if (ImGui::MenuItem("Camera"))
				{
					Camera* cam = Scene::current_scene.AddToScene(Camera("Camera"), Engine::Get().GameStateIsPlaying()).get();
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
			Object* obj = Scene::current_scene.GetSceneObject(front).get();
			if (obj != nullptr)
			{
				obj->Destroy();
			}
			remove_objects_queue.pop();
		}

		ImGui::End();
	}

}