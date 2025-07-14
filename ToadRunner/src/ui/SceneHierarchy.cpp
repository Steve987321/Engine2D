#include "pch.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "SceneHierarchy.h"

#include "engine/Engine.h"
#include "engine/PlaySession.h"

#include "MessageQueue.h"

#include "UI.h"

using json = nlohmann::ordered_json;

namespace Toad
{
    struct IterateChildrenState
    {
        std::set<std::string> scene_objects_set{};
        std::vector<std::string> scene_objects{};
        std::string drag_drop_extra_check_parent;
        std::string drag_drop_extra_check_child;
        size_t prev_cursor_index;
        size_t cursor_index;
        bool ignore_mouse_click = false;
        bool cursor_index_is_under;
        bool check_range;
        int index;
    };

    static void IterateChildren(Object* obj, bool dontchangeskip, IterateChildrenState& state)
    {
        state.index++;
        static bool skip = false;

        const auto drag_drop = [&state](bool& skip, Object* obj) {
            if (ImGui::BeginDragDropTarget())
            {
                if (ui::selected_obj && obj)
                    LOGDEBUGF("dragging {1} to {0}", obj->name, ui::selected_obj->name);
                
                if (obj)
                    state.drag_drop_extra_check_parent = obj->name;
                if (ui::selected_obj)
                    state.drag_drop_extra_check_child = ui::selected_obj->name;

                bool move_object = ImGui::AcceptDragDropPayload("move object");
                if (move_object)
                {
                    if (ui::selected_obj != nullptr)
                    {
                        ui::selected_obj->SetParent(obj);
                    }
                    for (const std::string& o : ui::selected_objects)
                    {
                        Object* as_object = Scene::current_scene.GetSceneObject(o).get();

                        if (as_object != nullptr)
                            as_object->SetParent(obj);
                    }
                }
                ImGui::EndDragDropTarget();
            }
            if (ImGui::BeginDragDropSource())
            {
                if (!skip)
                {
                    if (ui::selected_obj != nullptr)
                    {
                        if (!ui::selected_objects.contains(obj->name) && ui::selected_obj->name != obj->name)
                        {
                            ui::selected_objects.clear();
                            ui::selected_obj = obj;
                        }
                        ImGui::SetDragDropPayload("move object", obj->name.c_str(), obj->name.length());
                    }
                    else
                    {
                        ui::selected_obj = obj;
                    }
                }
                skip = true;
                ImGui::EndDragDropSource();
            }
        };

        if (!state.scene_objects_set.contains(obj->name))
        {
            state.scene_objects.emplace_back(obj->name);
            state.scene_objects_set.emplace(obj->name);
        }

        if (obj->GetChildren().empty())
        {
            if (ImGui::Selectable(obj->name.c_str(), ui::selected_objects.contains(obj->name) || (ui::selected_obj != nullptr && ui::selected_obj->name == obj->name)))
            {
                if ((ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_ModSuper)) && ui::selected_obj != nullptr && ui::selected_obj->name != obj->name)
                {
                    if (ui::selected_objects.contains(obj->name))
                    {
                        ui::selected_objects.erase(obj->name);
                    }
                    else
                    {
                        ui::selected_objects.insert(obj->name);
                    }
                }
                else if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && ui::selected_obj != nullptr && ui::selected_obj->name != obj->name)
                {
                    state.check_range = true;
                    state.cursor_index = state.index;
                    state.cursor_index_is_under = state.cursor_index > state.prev_cursor_index;
                }
                else
                {
                    state.prev_cursor_index = state.index;
                    ui::selected_obj = obj;
                    ui::selected_objects.clear();
                }
            }
            if (ImGui::IsItemHovered())
            {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    if (ui::selected_objects.empty())
                    {
                        ui::selected_obj = obj;
                    }
                    state.prev_cursor_index = state.index;

                    ImGui::PushOverrideID(ui::scene_modify_popup_id);
                    ImGui::OpenPopup("SceneModifyPopup");
                    ImGui::PopID();

                    state.ignore_mouse_click = true;
                }
            }
        }
        else
        {
            ImGuiTreeNodeFlags node_flags = 0;
            node_flags |= ImGuiTreeNodeFlags_OpenOnArrow;
            if (ui::selected_objects.contains(obj->name) || (ui::selected_obj != nullptr && ui::selected_obj->name == obj->name))
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
                        if (ui::selected_objects.empty())
                        {
                            ui::selected_obj = obj;
                        }
                        state.prev_cursor_index = state.index;
                        if (!ImGui::IsPopupOpen("SceneModifyPopup"))
                        {
                            ImGui::PushOverrideID(ui::scene_modify_popup_id);
                            ImGui::OpenPopup("SceneModifyPopup");
                            ImGui::PopID();
                            state.ignore_mouse_click = true;
                        }
                    }
                    else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                    {
                        if ((ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_ModSuper)) && ui::selected_obj != nullptr && ui::selected_obj->name != obj->name)
                        {
                            if (ui::selected_objects.contains(obj->name))
                                ui::selected_objects.erase(obj->name);
                            else
                                ui::selected_objects.insert(obj->name);
                        }
                        else if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && ui::selected_obj != nullptr && ui::selected_obj->name != obj->name)
                        {
                            state.check_range = true;
                            state.cursor_index = state.index;
                            state.cursor_index_is_under = state.cursor_index > state.prev_cursor_index;
                        }
                        else
                        {
                            state.prev_cursor_index = state.index;
                            ui::selected_obj = obj;
                            ui::selected_objects.clear();
                        }
                    }
                }

                if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                    drag_drop(skip, obj);

                for (Object* child : obj->GetChildrenAsObjects())
                {
                    if (!state.scene_objects_set.contains(child->name))
                    {
                        state.scene_objects.emplace_back(child->name);
                        state.scene_objects_set.emplace(child->name);
                    }

                    IterateChildren(child, true, state);

                    if (ImGui::IsItemHovered())
                    {
                        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                        {
                            if (ui::selected_objects.empty())
                                ui::selected_obj = child;

                            state.prev_cursor_index = state.index;
                            if (!ImGui::IsPopupOpen("SceneModifyPopup"))
                            {
                                ImGui::PushOverrideID(ui::scene_modify_popup_id);
                                ImGui::OpenPopup("SceneModifyPopup");
                                ImGui::PopID();
                                state.ignore_mouse_click = true;
                            }
                        }
                        else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                        {
                            if ((ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_ModSuper)) && ui::selected_obj != nullptr && ui::selected_obj->name != child->name)
                            {
                                if (ui::selected_objects.contains(child->name))
                                    ui::selected_objects.erase(child->name);
                                else
                                    ui::selected_objects.insert(child->name);
                            }
                            else if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && ui::selected_obj != nullptr && ui::selected_obj->name != child->name)
                            {
                                state.check_range = true;
                                state.cursor_index = state.index;
                                state.cursor_index_is_under = state.cursor_index > state.prev_cursor_index;
                            }
                            else
                            {
                                state.prev_cursor_index = state.index;
                                ui::selected_obj = child;
                                skip = true;
                                ui::selected_objects.clear();
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
                        if (ui::selected_objects.empty())
                        {
                            ui::selected_obj = obj;
                        }
                        state.prev_cursor_index = state.index;
                        if (!ImGui::IsPopupOpen("SceneModifyPopup"))
                        {
                            ImGui::PushOverrideID(ui::scene_modify_popup_id);
                            ImGui::OpenPopup("SceneModifyPopup");
                            ImGui::PopID();
                            state.ignore_mouse_click = true;
                        }
                    }
                    else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                    {
                        if ((ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_ModSuper)) && ui::selected_obj != nullptr && ui::selected_obj->name != obj->name)
                        {
                            if (ui::selected_objects.contains(obj->name))
                            {
                                ui::selected_objects.erase(obj->name);
                            }
                            else
                            {
                                ui::selected_objects.insert(obj->name);
                            }
                        }
                        else if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && ui::selected_obj != nullptr && ui::selected_obj->name != obj->name)
                        {
                            state.check_range = true;
                            state.cursor_index = state.index;
                            state.cursor_index_is_under = state.cursor_index > state.prev_cursor_index;
                        }
                        else
                        {
                            state.prev_cursor_index = state.index;
                            ui::selected_obj = obj;
                            ui::selected_objects.clear();
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
    }

	void ShowSceneHierarchy(MessageQueue& message_queue)
	{
		// make sure this is only true on click
		hierarchy_clicked_object = false;
        
        std::vector<std::pair<std::string, std::string>> set_object_childs = {};
        
        static IterateChildrenState iterate_children_state;
        iterate_children_state.index = 0;
        iterate_children_state.scene_objects.clear();
        iterate_children_state.scene_objects_set.clear();

		ImGui::Begin("Scene", nullptr);

		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_ModSuper))
		{
			Scene& scene = Scene::current_scene;

			if (ImGui::IsKeyPressed(ImGuiKey_S))
			{
				if (!scene.path.empty())
				{
					ui::saved_scene = true;
					SaveScene(scene, scene.path.parent_path());
                    
                    MessageQueueMessage msg;
                    msg.msg = "Saved scene";
                    msg.category = MessageCategory::GAME;
                    msg.show_time_ms = 2000;
                    msg.type = MessageType::INFO;

                    message_queue.AddToMessageQueue(msg);
				}
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_C))
			{
				std::vector<Object*> objects;
				size_t count = 0;
				if (ui::selected_obj != nullptr)
				{
					count++;
					objects.emplace_back(ui::selected_obj);
				}
				count += ui::selected_objects.size();

				objects.reserve(count);

				for (const std::string& name : ui::selected_objects)
					objects.emplace_back(scene.GetSceneObject(name).get());

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
					LoadSceneObjects(data_json, scene, "asset_browser.GetAssetPath()", false, true);
				}
				catch (const json::parse_error& e)
				{
					LOGERRORF("json parse error {}, check your clipboard", e.what());
				}
			}
		}

		ImGui::SeparatorText((ui::saved_scene ? Scene::current_scene.name : Scene::current_scene.name + '*').c_str());
		std::queue<std::string> remove_objects_queue;
		
		if (ImGui::IsKeyPressed(ImGuiKey_Delete))
		{
			if (ui::selected_obj != nullptr)
			{
				ui::selected_obj->Destroy();
				ui::selected_obj = nullptr;
			}
			for (const std::string& s : ui::selected_objects)
			{
				Scene::current_scene.GetSceneObject(s)->Destroy();
				remove_objects_queue.emplace(s);
			}
		}

		for (auto& obj : Scene::current_scene.objects_all)
		{
			// make sure we only iterate over root objects
			if (!obj->GetParent().empty())
				continue;

			IterateChildren(obj.get(), false, iterate_children_state);
		}

		// do some extra checks beucase the IterateChildren is not good 
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
            auto& drag_drop_extra_check_parent = iterate_children_state.drag_drop_extra_check_parent;
            auto& drag_drop_extra_check_child = iterate_children_state.drag_drop_extra_check_child;
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
                    
                    child_obj->SetParent(parent_obj.get());
				}
			}
		}

		if (iterate_children_state.check_range)
		{
			std::vector<std::string> keys;
			int origin = 0;
			int i = 0;
			for (const std::string& name : iterate_children_state.scene_objects)
			{
				i++;
				if (name == ui::selected_obj->name)
					origin = i;

				keys.push_back(name);
			}

			//LOGDEBUGF("prev_index={} is_under={} cursor_index={} origin={}", prev_cursor_index, cursor_index_is_under, cursor_index, origin);
			iterate_children_state.cursor_index = std::clamp(iterate_children_state.cursor_index, (size_t)0, keys.size());

			ui::selected_objects.clear();
			int dest = 0;
			if (iterate_children_state.cursor_index_is_under)
			{
				dest = static_cast<int>(iterate_children_state.cursor_index);
			}
			else
			{
				dest = static_cast<int>(iterate_children_state.cursor_index);
				origin -= 2;
			}
			for (int i = origin; iterate_children_state.cursor_index_is_under ? i < dest : i >= dest - 1; iterate_children_state.cursor_index_is_under ? i++ : i--)
			{
				LOGDEBUGF("{} {}", i, keys[i]);
				ui::selected_objects.insert(keys[i]);
			}

			iterate_children_state.check_range = false;
		}

		bool hovering_scene_hierarchy = ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize());
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			if (!ImGui::IsAnyItemHovered() && hovering_scene_hierarchy)
			{
				ui::selected_obj = nullptr;
				ui::selected_objects.clear();
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

		if (!iterate_children_state.ignore_mouse_click && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
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
					Scene::current_scene.AddToScene(Object("Object"), Toad::begin_play);

				if (ImGui::MenuItem("Circle"))
					Scene::current_scene.AddToScene(Circle("Circle"), Toad::begin_play);

				if (ImGui::MenuItem("Sprite"))
					Scene::current_scene.AddToScene(Sprite("Sprite"), Toad::begin_play);

				if (ImGui::MenuItem("Audio"))
					Scene::current_scene.AddToScene(Audio("Audio"), Toad::begin_play);

				if (ImGui::MenuItem("Text"))
					Scene::current_scene.AddToScene(Text("Text"), Toad::begin_play);

				if (ImGui::MenuItem("Camera"))
				{
					Camera* cam = Scene::current_scene.AddToScene(Camera("Camera"), Toad::begin_play).get();
					cam->ActivateCamera();
				}

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		ImGui::PushOverrideID(ui::scene_modify_popup_id);
		if (ImGui::BeginPopup("SceneModifyPopup"))
		{
            if (ImGui::MenuItem("Delete"))
            {
                if (ui::selected_obj != nullptr)
                    remove_objects_queue.push(ui::selected_obj->name);

                if (!ui::selected_objects.empty())
                {
                    for (const std::string& name : ui::selected_objects)
                        remove_objects_queue.push(name);
                }
            }
            if (ImGui::MenuItem("UnParent"))
            {
                if (ui::selected_obj != nullptr)
                    ui::selected_obj->SetParent(nullptr);

                if (!ui::selected_objects.empty())
                {
                    for (const std::string& name : ui::selected_objects)
                    {
                        auto obj = Scene::current_scene.GetSceneObject(name);
                        if (obj)
                            obj->SetParent(nullptr);
                    }
                }
            }

			ImGui::EndPopup();
		}
		ImGui::PopID();

		while (!remove_objects_queue.empty())
		{
			const std::string& front = remove_objects_queue.front();
			ui::selected_obj = nullptr;
			ui::selected_objects.erase(front);
			auto obj = Scene::current_scene.GetSceneObject(front);
			if (obj != nullptr)
				obj->Destroy();
                
			remove_objects_queue.pop();
		}

		ImGui::End();
	}

}
