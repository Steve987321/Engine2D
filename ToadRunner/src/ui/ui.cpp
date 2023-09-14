#include "pch.h"

#include "Engine/Engine.h"
#include "ui.h"

#include "imgui/imgui-SFML.h"

#include <queue>

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

	constexpr auto dock_window_flags =
		ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiButtonFlags_NoNavFocus;

	ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos);

	ImGui::Begin("DockSpace", nullptr, dock_window_flags);
	ImGui::DockSpace(ImGui::GetID("DockSpace"));
	ImGui::End();

	ImGui::Begin("Settings", nullptr);
	{
		ImGui::Text("FPS %.1f", 1.f / Toad::Engine::Get().GetDeltaTime().asSeconds());

		ImGui::End();
	}

	static std::shared_ptr<Toad::Object> selected_obj = nullptr;
	static std::shared_ptr<Toad::Sprite> selected_sprite = nullptr;

	ImGui::Begin("Scene", nullptr);
	{
		std::queue<std::string> removeObjQueue{};

		if (ImGui::Button("Add Sprite"))
		{
			Toad::Engine::Get().GetScene().AddToScene(Toad::Sprite("Sprite"));
		}
		if (ImGui::Button("Add Circle"))
		{
			Toad::Engine::Get().GetScene().AddToScene(Toad::Circle("Circle"));
		}

		for (auto& [name, obj] : Toad::Engine::Get().GetScene().objectsMap)
		{
			if (ImGui::Selectable(name.c_str(), selected_obj != nullptr && selected_obj->name == name))
			{
				selected_obj = obj;
			}
		}

		while (!removeObjQueue.empty())
		{
			Toad::Engine::Get().GetScene().RemoveFromScene(removeObjQueue.front());
			removeObjQueue.pop();
		}

		ImGui::End();
	}

	ImGui::Begin("Inspector", nullptr);
	{
		if (selected_obj != nullptr)
		{
			ImGui::Text(selected_obj->name.c_str());
			for (const auto& script : selected_obj->GetAttachedScripts())
			{
				ImGui::Selectable(script.GetName().c_str());
			}
			auto spriteObj = dynamic_cast< Toad::Sprite* >(selected_obj.get());
			auto circleObj = dynamic_cast< Toad::Circle* >(selected_obj.get());
			if (spriteObj != nullptr)
			{
				auto& sprite = spriteObj->GetSprite();
				
				auto pos = sprite.getPosition();

				if (ImGui::DragFloat("X", &pos.x))
					sprite.setPosition(pos);
				if (ImGui::DragFloat("Y", &pos.y))
					sprite.setPosition(pos);

				auto colorU32 = ImGui::ColorConvertU32ToFloat4(sprite.getColor().toInteger());
				float col[4] = { colorU32.x, colorU32.y, colorU32.z, colorU32.w };
				if (ImGui::ColorEdit4("color", col))
					sprite.setColor(sf::Color(ImGui::ColorConvertFloat4ToU32({col[0], col[1], col[2], col[3]})));
			}
			else if (circleObj != nullptr)
			{
				auto& circle = circleObj->GetCircle();

				auto pos = circle.getPosition();

				if (ImGui::DragFloat("X", &pos.x))
					circle.setPosition(pos);
				if (ImGui::DragFloat("Y", &pos.y))
					circle.setPosition(pos);

				auto colorU32 = ImGui::ColorConvertU32ToFloat4(circle.getFillColor().toInteger());
				float col[4] = { colorU32.x, colorU32.y, colorU32.z, colorU32.w };
				if (ImGui::ColorEdit4("color", col))
					circle.setFillColor(sf::Color(ImGui::ColorConvertFloat4ToU32({ col[0], col[1], col[2], col[3] })));

				auto circleRadius = circle.getRadius();
				if (ImGui::DragFloat("radius", &circleRadius))
					circle.setRadius(circleRadius);
			}
		}

		ImGui::End();
	}

	ImGui::Begin("Viewport", nullptr);
	{
		auto& window_texture = Toad::Engine::Get().GetWindowTexture();

		ImGui::Image(window_texture, {1920, 1080}, sf::Color::White);
		
		ImGui::End();
	}
}
