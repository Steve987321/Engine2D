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

	FSMGraphEditor::FSMGraphEditor()
	{
		m_inspectorUI = std::bind(&FSMGraphEditor::ShowFSMProps, this);
	}

	void FSMGraphEditor::Show(bool* show, const GameAssetsBrowser& asset_browser)
	{
		ImGui::Begin("FSMGraphEditor", show, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_HorizontalScrollbar);
		{
			if (ImGui::IsWindowFocused())
				if (ui::inspector_ui.target_type() != m_inspectorUI.target_type())
						ui::inspector_ui = m_inspectorUI;

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
				FSMGraphEditorNodeInfo::node_prev_offset_pos = ImGui::GetCursorPos();
				ImGui::SmallButton("I");
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
					ImGui::GetForegroundDrawList()->AddLine(press_pos, press_pos + drag_delta, IM_COL32(255, 255, 255, 255), 2.f);
				}
				else
					next = false;
			}

			if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.f))
			{
				ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle, 0.f);
				ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
				ImVec2 current_scroll = { ImGui::GetScrollX(), ImGui::GetScrollY() };
				ImGui::SetScrollX(current_scroll.x + drag_delta.x);
				ImGui::SetScrollY(current_scroll.y + drag_delta.y);
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

	void FSMGraphEditor::ShowFSMProps()
	{
		if (!fsm)
		{
			ImGui::Text("no fsm please create one");
			return;
		}
		
		ImGui::Text(fsm->GetName().c_str());
		ImGui::Separator();

		if (ImGui::TreeNode("FSM Variables"))
		{
			if (ImGui::TreeNode("I32"))
			{
				for (FSMVariable<int>& var : fsm->varsi32)
				{
					char name_buf[32];
					strncpy(name_buf, var.name.c_str(), 32);
					bool found = false;
					if (ImGui::InputText("name", name_buf, 32))
					{
						for (FSMVariable<int>& var_other : fsm->varsi32)
						{
							if (&var == &var_other)
								continue;
							if (var_other.name == name_buf)
								found = true;
						}
					}
					if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
					{
						if (!found)
							var.name = name_buf;
					}

					ImGui::DragInt(var.name.c_str(), &var.data);
				}

				if (ImGui::Button("ADD I32"))
					fsm->AddVariable("var", 0);

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("FLT"))
			{
				for (FSMVariable<float>& var : fsm->varsflt)
				{
					char name_buf[32];
					strncpy(name_buf, var.name.c_str(), 32);
					bool found = false;

					if (ImGui::InputText("name", name_buf, 32))
					{
						for (FSMVariable<float>& var_other : fsm->varsflt)
						{
							if (&var == &var_other)
								continue;
							if (var_other.name == name_buf)
								found = true;
						}
					}
					if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
					{
						if (!found)
							var.name = name_buf;
					}

					ImGui::DragFloat(var.name.c_str(), &var.data);
				}

				if (ImGui::Button("ADD FLT"))
					fsm->AddVariable("var", 0.f);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		std::vector<State>& states = fsm->GetStates();

		if (ImGui::TreeNode("States", "States (%d)", states.size()))
		{
			for (int i = 0; i < states.size(); i++)
			{
				State& state = states[i];

				if (ImGui::TreeNode(std::to_string(i).c_str(), "(%s)", state.name.c_str()))
				{
					for (int j = 0; j < state.transitions.size(); j++)
					{
						Transition& transition = state.transitions[j];

						if (ImGui::TreeNode((std::to_string(j) + "i32cond").c_str(), "condition (%d) (i32)", j))
						{
							for (TransitionCondition& cond : transition.conditions_i32)
							{
								cond.a;
								cond.b;
								if (cond.a > fsm->varsi32.size() || cond.b > fsm->varsi32.size())
								{
									ImGui::Text("Condition variables are invalid");
								}
								else
								{
									FSMVariable<int> var_a = fsm->varsi32[cond.a];
									FSMVariable<int> var_b = fsm->varsi32[cond.b];
									ImGui::DragInt(var_a.name.c_str(), &var_a.data);
									ImGui::DragInt(var_b.name.c_str(), &var_b.data);

									char compare_str[2];
									to_string(cond.comparison_type, compare_str);

									if (ImGui::BeginCombo("click check", compare_str))
									{
										for (int i = 0; i < (int)CompareType::COUNT - 1; i++)
										{
											CompareType ctype = static_cast<CompareType>(i);
											to_string(ctype, compare_str);

											if (ImGui::Selectable(compare_str, ctype == cond.comparison_type))
												cond.comparison_type = ctype;
										}
										ImGui::EndCombo();
									}
								}

							}

							if (ImGui::Button("ADD CONDITION I32"))
							{
								if (fsm->varsi32.empty())
									LOGERRORF("[FSMGraphEditor] Can't add i32 condition, fsm doesn't have any float variables");
								else
								{
									TransitionCondition cond(fsm, 0, 0, FSMVariableType::INT32, CompareType::EQUAL);
									transition.AddConditionI32(cond);
								}

							}
							ImGui::TreePop();
						}

						if (ImGui::TreeNode((std::to_string(j) + "fltcond").c_str(), "condition (%d) (flt)", j))
						{
							for (TransitionCondition& cond : transition.conditions_flt)
							{
								cond.a;
								cond.b;
								if (cond.a > fsm->varsflt.size() || cond.b > fsm->varsflt.size())
								{
									ImGui::Text("Condition variables are invalid");
								}
								else
								{
									FSMVariable<float> var_a = fsm->varsflt[cond.a];
									FSMVariable<float> var_b = fsm->varsflt[cond.b];
									ImGui::DragFloat(var_a.name.c_str(), &var_a.data);
									ImGui::DragFloat(var_b.name.c_str(), &var_b.data);
									
									char compare_str[2];
									to_string(cond.comparison_type, compare_str);

									if (ImGui::BeginCombo("click check", compare_str))
									{
										for (int i = 0; i < (int)CompareType::COUNT - 1; i++)
										{
											CompareType ctype = static_cast<CompareType>(i);
											to_string(ctype, compare_str);

											if (ImGui::Selectable(compare_str, ctype == cond.comparison_type))
												cond.comparison_type = ctype;
										}
										ImGui::EndCombo();
									}
								}

							}

							if (ImGui::Button("ADD CONDITION FLT"))
							{
								if (fsm->varsflt.empty())
									LOGERRORF("[FSMGraphEditor] Can't add float condition, fsm doesn't have any float variables");
								else
								{
									TransitionCondition cond(fsm, 0, 0, FSMVariableType::FLOAT, CompareType::EQUAL);
									transition.AddConditionFlt(cond);
								}

							}
							ImGui::TreePop();
						}
					}

					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
	}

}