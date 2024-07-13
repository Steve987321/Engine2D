#include "pch.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "Engine/Engine.h"

#include "FSMGraphEditor.h"

#include "engine/utils/FSM.h"
#include "project/ToadProject.h"

#include "UI/GameAssetsBrowser.h"
#include "UI.h"
#include "imgui-SFML.h"

namespace Toad
{

	namespace fs = std::filesystem;

	void FSMGraphEditor::Show(bool* show, const GameAssetsBrowser& asset_browser)
	{
		ImGui::Begin("FSMGraphEditor", show, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_HorizontalScrollbar);
		{
			if (!fsm)
				return;

			bool save_to_file = false;
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::MenuItem("Save", "CTRL+S"))
					save_to_file = true;

				ImGui::EndMenuBar();
			}
			if (save_to_file)
				SaveToFile(asset_browser.GetAssetPath() / (fsm->GetName() + FILE_EXT_FSM));

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
				ImGui::OpenPopup("AddStatePopup");

			if (ImGui::BeginPopup("AddStatePopup"))
			{
				if (ImGui::Selectable("Add State"))
				{
					fsm->AddState(State("state"));
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			for (State& state : fsm->GetStates())
			{
				ImGui::SetCursorPos(_statePos[state.name]);
				ImGui::BeginChild(state.name.c_str(), {0, 0}, true);
				ImGui::Text(state.name.c_str());
				ImGui::EndChild();

				if (ImGui::IsItemHovered())
				{
					if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
					{
						ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
						_statePos[state.name] += drag_delta;
					}
				}
			}

			if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
			{
				ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
				ImVec2 current_scroll = {ImGui::GetScrollX(), ImGui::GetScrollY()};
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
			_statePos[item.key()] = {x,y};
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

		for (const auto& [name, pos] : _statePos)
		{
			json pos_data;
			pos_data["x"] = pos.x;
			pos_data["y"] = pos.y;
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