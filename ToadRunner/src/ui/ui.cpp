#include "pch.h"

#include "Engine/Engine.h"
#include "ui.h"

#include "imgui-SFML.h"

#include <queue>
#include <limits>
#include <string>

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
	if (ImGui::BeginMenuBar())
	{

		ImGui::EndMenuBar();
	}
	ImGui::End();

	ImGui::Begin("Settings", nullptr);
	{
		ImGui::Text("FPS %.1f", 1.f / Toad::Engine::Get().GetDeltaTime().asSeconds());

		if (ImGui::Button("Save scene"))
		{
			static char scene_path[MAX_PATH] = "";
			ImGui::InputText("path", scene_path, MAX_PATH);
			Toad::SaveScene(Toad::Engine::Get().GetScene(), scene_path);
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

	static std::shared_ptr<Toad::Object> selected_obj = nullptr;
	static std::shared_ptr<Toad::Sprite> selected_sprite = nullptr;

	ImGui::Begin("Scene", nullptr);
	{
		std::queue<std::string> remove_obj_queue{};

		if (ImGui::Button("Add Sprite"))
		{
			Toad::Engine::Get().GetScene().AddToScene(Toad::Sprite("Sprite"));
		}
		if (ImGui::Button("Add Circle"))
		{
			Toad::Engine::Get().GetScene().AddToScene(Toad::Circle("Circle"));
		}

		for (auto& [name, obj] : Toad::Engine::Get().GetScene().objects_map)
		{
			if (ImGui::Selectable(name.c_str(), selected_obj != nullptr && selected_obj->name == name))
			{
				selected_obj = obj;
			}
		}

		while (!remove_obj_queue.empty())
		{
			Toad::Engine::Get().GetScene().RemoveFromScene(remove_obj_queue.front());
			remove_obj_queue.pop();
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

				auto pos = sprite.getPosition();

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

				auto circle_col = circle.getFillColor();
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
						selected_obj->RemoveScript(selected_script_name.c_str());
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
}
