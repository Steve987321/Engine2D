#include "pch.h"

#include "Engine/Engine.h"
#include "ui.h"

void ui::decorations()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	style->FrameRounding = 3.f;
	style->GrabRounding = 2.f;

	//main colors 21 21 21, 36 36 36, 48 173 228
	ImU32 dark_gray = ImColor(21, 21, 21);
	ImU32 gray = ImColor(36, 36, 36);
	ImU32 light_gray = ImColor(46, 46, 46);
	ImU32 light_blue = ImColor(48, 173, 228);
	ImU32 lighter_blue = ImColor(147, 170, 180);
	ImU32 dark_blue = ImColor(147, 170, 180);
	ImU32 darker_white = ImColor(184, 184, 184);

	style->Colors[ImGuiCol_Button] = (ImColor)gray;
	style->Colors[ImGuiCol_ButtonHovered] = (ImColor)light_gray;
	style->Colors[ImGuiCol_ButtonActive] = ImColor(13, 13, 13);

	style->Colors[ImGuiCol_CheckMark] = (ImColor)light_blue;

	style->Colors[ImGuiCol_FrameBg] = (ImColor)gray;
	style->Colors[ImGuiCol_FrameBgHovered] = (ImColor)gray;
	style->Colors[ImGuiCol_FrameBgActive] = ImColor(13, 13, 13);

	style->Colors[ImGuiCol_TitleBg] = (ImColor)dark_gray;
	style->Colors[ImGuiCol_TitleBgActive] = (ImColor)light_blue;

	style->Colors[ImGuiCol_ScrollbarBg] = ImColor(15, 15, 15);
	style->Colors[ImGuiCol_ScrollbarGrab] = (ImColor)gray;
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = (ImColor)gray;
	style->Colors[ImGuiCol_ScrollbarGrabActive] = (ImColor)gray;

	style->Colors[ImGuiCol_SliderGrab] = (ImColor)gray;
	style->Colors[ImGuiCol_SliderGrabActive] = (ImColor)gray;

	style->Colors[ImGuiCol_Text] = (ImColor)darker_white;

	style->Colors[ImGuiCol_WindowBg] = (ImColor)dark_gray;

	style->Colors[ImGuiCol_HeaderHovered] = ImColor(55, 55, 55);
	style->Colors[ImGuiCol_HeaderActive] = (ImColor)light_gray;
	style->Colors[ImGuiCol_Header] = (ImColor)lighter_blue;
}

std::once_flag flag;

void ui::engine_ui(ImGuiContext* ctx)
{
	ImGui::SetCurrentContext(ctx);

	decorations();

	std::call_once(flag, []()
		{
			ImGui::SetNextWindowSize(ImVec2(340, 100));
			ImGui::SetNextWindowPos(ImVec2(0, 0));
		});

	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
	{
		ImGui::Text("FPS %.1f", 1.f / Toad::Engine::Get().GetDeltaTime().asSeconds());

		std::queue<Toad::Object> removeObjQueue{};
		for (const auto& [name, obj] : Toad::Engine::Get().GetScene().objectsMap)
		{
			
		}

		while (!removeObjQueue.empty())
		{
			Toad::Engine::Get().GetScene().AddToScene()
		}

		ImGui::End();
	}
}
