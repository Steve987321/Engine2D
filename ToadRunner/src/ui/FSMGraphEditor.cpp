#include "pch.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "Engine/Engine.h"

#include "FSMGraphEditor.h"

#include "engine/utils/FSM.h"
#include "project/ToadProject.h"

#include "ui/GameAssetsBrowser.h"
#include "UI.h"
#include "imgui-SFML.h"

#include "utils/FileDialog.h"

namespace Toad
{

	namespace fs = std::filesystem;

	void FSMGraphEditor::Show(bool* show, const GameAssetsBrowser& asset_browser)
	{
		ImGui::Begin("FSMGraphEditor", show, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_HorizontalScrollbar);
		{
			bool save_to_file = false;
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::MenuItem("Save", "CTRL+S"))
					save_to_file = true;

				if (ImGui::MenuItem("Load"))
					GetPathFile(asset_browser.GetAssetPath().string(), "Finite State Machine (*.FSM)\0*.FSM\0");

				ImGui::EndMenuBar();
			}
			if (save_to_file)
			{
				SaveToFile(asset_browser.GetAssetPath() / (fsm->GetName() + FILE_EXT_FSM));
				save_to_file = false;
			}

			if (!fsm)
			{
				ImGui::Text("NO FSM NO FSM NO FSM");
				if (ImGui::Button("Create"))
				{
					const std::string name = "fsm";
					std::string fixed_name = name;
					std::string id = (asset_browser.GetAssetPath() / (name + FILE_EXT_FSM)).string();

					if (fs::exists(id))
					{
						int i = 1;
						while (fs::exists(id))
						{
							fixed_name = name + std::to_string(i);
							id = (asset_browser.GetAssetPath() / (fixed_name + FILE_EXT_FSM)).string();
						}
					}

					FSM new_fsm(fixed_name);
					fsm = Engine::Get().GetResourceManager().AddFSM(id, new_fsm);
				}
				ImGui::End();
				return;
			}

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
				ImGui::OpenPopup("AddStatePopup");

			if (ImGui::BeginPopup("AddStatePopup"))
			{
				if (ImGui::Selectable("Add State"))
				{
					fsm->AddState(State("state"));
					FSMGraphEditorNodeInfo info;
					info.pos = { ImGui::GetWindowPos() - ImGui::GetMousePos() };
					_statePos[fsm->GetStates().back().name] = info;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			static bool prev = false;
			static bool next = false;
			static ImVec2 press_pos = {};
			static bool is_dragging_state = false;
			static std::string dragging_state;

			std::vector<State>& fsm_states = fsm->GetStates();
			for (uint32_t i = 0; i < fsm_states.size(); i++)
			{
				State& state = fsm_states[i];

				// add node things for creating/linking transitions
				ImGui::SetCursorPos(_statePos[state.name].pos);
				
				ImGui::BeginChild(state.name.c_str(), { 150, 150 }, true);
				ImGui::Text(state.name.c_str());

				ImGui::PushID("prev");
				ImGui::SmallButton("I");
				FSMGraphEditorNodeInfo::node_prev_offset_pos = ImGui::GetCursorPos();
				ImGui::PopID();

				ImGui::SameLine(0, ImGui::GetWindowWidth() - ImGui::GetStyle().ItemSpacing.x - 40);
				
				ImGui::PushID("next");
				FSMGraphEditorNodeInfo::node_next_offset_pos = ImGui::GetCursorPos();
				ImGui::SmallButton("O");
				if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					next = true;
					press_pos = ImGui::GetMousePos();
				}
				ImGui::PopID();

				if (ImGui::BeginDragDropSource())
				{
					ImGui::SetDragDropPayload("DND_TRANSITION", &i, sizeof(uint32_t));
					ImGui::EndDragDropSource();
				}

				ImGui::EndChild();

				if (ImGui::BeginDragDropTarget())
				{
					const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_TRANSITION");
					if (payload && payload->Data)
					{
						uint32_t prev_state = *(uint32_t*)payload->Data;

						if (prev_state < fsm_states.size())
							fsm_states[prev_state].transitions.emplace_back(*fsm, prev_state, i);
						else
							LOGERRORF("[FSMGraphEditor] Dragged state index is invalid: {}", prev_state);
					}
					ImGui::EndDragDropTarget();
				}

				// transitions
				for (Transition& transition : state.transitions)
				{
					const FSMGraphEditorNodeInfo& p1 = _statePos[state.name];
					const FSMGraphEditorNodeInfo& p2 = _statePos[transition.GetNextState()->name];

					const ImVec2 window_pos = ImGui::GetWindowPos();

					const ImVec2& next_offset = FSMGraphEditorNodeInfo::node_next_offset_pos;
					const ImVec2& prev_offset = FSMGraphEditorNodeInfo::node_prev_offset_pos;
					ImGui::GetForegroundDrawList()->AddLine(p1.pos + next_offset + window_pos, p2.pos + prev_offset + window_pos, IM_COL32_WHITE, 2.f);
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					is_dragging_state = true;
					dragging_state = state.name;
				}
			}

			if (is_dragging_state)
			{
				if (ImGui::IsMouseDown(ImGuiMouseButton_Left, 0.f))
				{
					ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 0.f);
					ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
					_statePos[dragging_state].pos += drag_delta;
				}
				else
					is_dragging_state = false;
			}
			if (prev)
			{
				if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.f))
				{
					ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 0.f);
					LOGDEBUGF("prev {} {} ", drag_delta.x, drag_delta.y);
					ImGui::GetForegroundDrawList()->AddLine(press_pos, press_pos + drag_delta, IM_COL32(255, 255, 255, 255), 2.f);
				}
				else
					prev = false;
			}
			else if (next)
			{
				if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.f))
				{
					ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 0.f);
					LOGDEBUGF("next {} {} ", drag_delta.x, drag_delta.y);
					ImGui::GetForegroundDrawList()->AddLine(press_pos, press_pos + drag_delta, IM_COL32(255, 255, 255, 255), 2.f);
				}
				else
				{
					next = false;
				}
			}

			if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.f))
			{
				ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle, 0.f);
				ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
				ImVec2 current_scroll = { ImGui::GetScrollX(), ImGui::GetScrollY() };
				ImGui::SetScrollX(current_scroll.x + drag_delta.x);
				ImGui::SetScrollY(current_scroll.y + drag_delta.y);
				LOGDEBUGF("{} {}", drag_delta.x, drag_delta.y);
			}

			ImGui::End();
		}
	}

	bool FSMGraphEditor::LoadFromFile(const fs::path& path, const GameAssetsBrowser& asset_browser)
	{
		using json = nlohmann::json;

		std::ifstream file(path);
		if (!file)
		{
			LOGERRORF("[UI:FSMGraphEditor] Can't open file '{}'", path);
			return false;
		}

		json data;
		try
		{
			json::parse(file);
		}
		catch (const json::parse_error& e)
		{
			LOGERRORF("[UI:FSMGraphEditor] JSON parse error at '{}', file: '{}'", e.byte, path);
			return false;
		}

		json fsm_data;
		json imgui_data;

		bool fsm_data_res = get_json_element(fsm_data, data, "fsm");
		bool imgui_data_res = get_json_element(imgui_data, data, "imgui");

		if (!fsm_data_res)
		{
			LOGWARNF("[FSMGraphEditor] No fsm element in data, file: '{}'", path);
			fsm_data = data;
		}

		fs::path relative = fs::relative(path, asset_browser.GetAssetPath());
		fsm = Engine::Get().GetResourceManager().AddFSM(relative.string(), FSM::Deserialize(fsm_data));

		if (!imgui_data_res)
			return true;

		// get metadata for imgui 
		_statePos.clear();
		for (const auto& item : imgui_data.items())
		{
			float x = 0.f;
			float y = 0.f;
			json pos = item.value().at("pos");
			x = pos.at("x");
			y = pos.at("y");
			_statePos[item.key()].pos = {x,y};
		}
		
		return true;
	}

	bool FSMGraphEditor::SaveToFile(const fs::path& path)
	{
		if (!fsm)
		{
			LOGERRORF("[FSMGraphEditor] Can't save as fsm is not valid");
			return false;
		}

		fs::path corrected_path = path;
		if (fs::is_directory(path))
			corrected_path /= fsm->GetName() + FILE_EXT_FSM;

		if (!fs::is_regular_file(path))
		{ 
			LOGERRORF("[FSMGraphEditor] Invalid path: '{}'", corrected_path);
			return false;
		}
		std::ofstream file(corrected_path);
		if (!file)
		{
			LOGERRORF("[FSMGraphEditor] Failed to create output file for fsm: '{}'", corrected_path);
			return false;
		}

		json imgui_data;

		for (const auto& [name, info] : _statePos)
		{
			json pos_data;
			pos_data["x"] = info.pos.x;
			pos_data["y"] = info.pos.y;
			imgui_data[name] = pos_data;
		}

		json fsm_data = fsm->Serialize();

		json data;
		data["fsm"] = fsm_data;
		data["imgui"] = imgui_data;
		file << data << std::endl;
		file.close();
		return true;
	}

}