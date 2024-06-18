#include "pch.h"
#include "FSMGraphEditor.h"

#include "Engine/Engine.h"
#include "engine/utils/FSM.h"

#include "UI.h"
#include "imgui-SFML.h"

namespace Toad
{

	void FSMGraphEditor::Show(bool* show)
	{
		ImGui::Begin("FSMGraphEditor", show);
		{
			ImGui::BeginChild("FSMInspector", { ImGui::GetContentRegionAvail().x / 3, 0 }, true);
			{
				
			}
			ImGui::EndChild();

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				if (ImGui::IsWindowHovered())
				{
					ImGui::OpenPopup("AddStatePopup");
				}
			}
			if (ImGui::BeginPopup("AddStatePopup"))
			{
				if (ImGui::Selectable("Add State"))
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			ImGui::End();
		}
	}

}