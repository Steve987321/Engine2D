#include "pch.h"
#include "Inspector.h"

#include "engine/Engine.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ui/UI.h"
#include "imgui-SFML.h"
#include "ui/GameAssetsBrowser.h"

namespace ui
{

void object_inspector(Toad::Object*& selected_obj, const ui::GameAssetsBrowser& asset_browser)
{
	using namespace Toad;
	if (!selected_obj)
		return; 

	const auto& attached_scripts = selected_obj->GetAttachedScripts();
	bool suggestion = false;
	char name_buf[64];
	std::string new_name_str;
	strncpy(name_buf, selected_obj->name.c_str(), selected_obj->name.length() + 1);

	ImVec2 input_name_pos = ImGui::GetCursorPos();
	if (ImGui::InputText("name", name_buf, sizeof(name_buf)))
	{
		Toad::Scene& scene = Scene::current_scene;

		new_name_str = name_buf;

		int count = 0;
		bool found = false;

		for (const auto& obj : scene.objects_all)
		{
			if (obj->name == new_name_str)
			{
				count++;
				found = true;
			}
		}

		if (new_name_str != selected_obj->name && found)
		{
			suggestion = true;

			new_name_str += " (" + std::to_string(count) + ')';
			auto it = std::find_if(scene.objects_all.begin(), scene.objects_all.end(), [&new_name_str](const std::shared_ptr<Toad::Object>& obj) {return obj->name == new_name_str; });
			while (it != scene.objects_all.end())
			{
				new_name_str += " (" + std::to_string(++count) + ')';
				it = std::find_if(scene.objects_all.begin(), scene.objects_all.end(), [&new_name_str](const std::shared_ptr<Toad::Object>& obj) {return obj->name == new_name_str; });
			}
		}
		else
		{
			suggestion = false;
		}
	}

	if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
	{
		if (new_name_str != selected_obj->name)
		{
			selected_obj->name = new_name_str;

			/*auto& map = Scene::current_scene.objects_map;
			if (!map.contains(new_name_str))
			{
				auto a = map.extract(selected_obj->name);
				a.key() = new_name_str;
				map.insert(std::move(a));

				selected_obj->name = new_name_str;
			}*/
		}
	}

	if (suggestion)
	{
		//ImGuiInputTextState* state = ImGui::GetInputTextState(ImGui::GetID("name"));
		std::string part_of_str = new_name_str.substr(strlen(name_buf), new_name_str.length() - strlen(name_buf));
		ImVec2 frame_pad = ImGui::GetStyle().FramePadding;
		ImVec2 text_size = ImGui::CalcTextSize(name_buf);
		ImGui::SetCursorPos({ input_name_pos.x + text_size.x + frame_pad.x, input_name_pos.y + frame_pad.y });
		ImGui::TextColored({ 1,1,1,0.4f }, "%s", part_of_str.c_str());
	}

	ImGui::Text("%s", selected_obj->name.c_str());

	static Toad::Object* prev_selected_obj = nullptr;
	static size_t index = 0;
	auto& obj_all = Scene::current_scene.objects_all;
	if (prev_selected_obj != selected_obj)
	{
		for (size_t i = 0; i < obj_all.size(); i++)
		{
			if (obj_all[i].get() == selected_obj)
			{
				index = i;
			}
		}
	}

	prev_selected_obj = selected_obj;

	ImGui::SameLine(0, 10.f);
	ImGui::BeginDisabled(index == 0);
	if (ImGui::ArrowButton("##MoveUp", ImGuiDir_Up))
	{
		std::swap(obj_all[index], obj_all[index - 1]);
		--index;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(index == obj_all.size() - 1);
	if (ImGui::ArrowButton("##MoveDown", ImGuiDir_Down))
	{
		std::swap(obj_all[index], obj_all[index + 1]);
		++index;
	}
	ImGui::EndDisabled();

	float pos_x = selected_obj->GetPosition().x;
	float pos_y = selected_obj->GetPosition().y;
	float rotation = selected_obj->GetRotation();

	if (ImGui::DragFloat("x", &pos_x))
	{
		selected_obj->SetPosition({ pos_x, pos_y });
	}
	if (ImGui::DragFloat("y", &pos_y))
	{
		selected_obj->SetPosition({ pos_x, pos_y });
	}
	if (ImGui::DragFloat("rotation (degrees)", &rotation))
	{
		selected_obj->SetRotation(rotation);
	}

	//if (ImGui::Button("Test"))
	//{
	//	auto child_obj = Scene::current_scene.AddToScene(Toad::Circle("child object"));
	//	child_obj->SetParent(selected_obj);
	//}

	ImGui::SeparatorText("object properties");

	auto sprite_obj = dynamic_cast<Toad::Sprite*>(selected_obj);
	auto circle_obj = dynamic_cast<Toad::Circle*>(selected_obj);
	auto audio_obj = dynamic_cast<Toad::Audio*>(selected_obj);
	auto text_obj = dynamic_cast<Toad::Text*>(selected_obj);
	auto cam_obj = dynamic_cast<Toad::Camera*>(selected_obj);

	if (sprite_obj != nullptr)
	{
		auto& sprite = sprite_obj->GetSprite();

		const sf::Texture& attached_texture = sprite.getTexture();

		ImGui::Text("texture");
		ImGui::SameLine();
	
		ImGui::Button("texture dragger", { 25, 25 });

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("move file");
			if (payload != nullptr)
			{
				std::filesystem::path src = *(std::string*)payload->Data;
				do
				{
					if (!src.has_extension() || (src.extension().string() != ".jpg" && src.extension().string() != ".png"))
					{
						break;
					}

					std::filesystem::path relative = std::filesystem::relative(src, asset_browser.GetAssetPath());
					if (ResourceManager::GetTextures().GetData().contains(relative.string()))
					{
						sf::Texture* managed_texture = ResourceManager::GetTextures().Get(relative.string());
						sprite_obj->SetTexture(relative, managed_texture);
					}
					else
					{
						sf::Texture new_texture;
						if (!new_texture.loadFromFile(src.string()))
						{
							LOGERRORF("Failed to load texture from {}", src);
						}
						sf::Texture* managed_texture = ResourceManager::GetTextures().Add(relative.string(), new_texture);
						if (managed_texture != nullptr)
						{
							sprite_obj->SetTexture(relative, managed_texture);
						}
					}

				} while (false);
			}

			ImGui::EndDragDropTarget();
		}

		if (!sprite_obj->HasDefaultTexture())
		{
			if (ImGui::TreeNode("texture rect"))
			{
				sf::IntRect rect = sprite.getTextureRect();
				if (ImGui::DragIntRect("sprite rect", &rect))
				{
					sprite.setTextureRect(rect);
				}

				ImGui::TreePop();
			}
		}

		const sf::Color& sprite_col = sprite.getColor();
		float col[4] = {
			sprite_col.r / 255.f,
			sprite_col.g / 255.f,
			sprite_col.b / 255.f,
			sprite_col.a / 255.f
		};
		if (ImGui::ColorEdit4("color", col))
			sprite.setColor(sf::Color{
			(uint8_t)(col[0] * 255.f),
			(uint8_t)(col[1] * 255.f),
			(uint8_t)(col[2] * 255.f),
			(uint8_t)(col[3] * 255.f) });

		Vec2f scale = sprite.getScale();
		if (ImGui::DragFloat("scale x", &scale.x))
		{
			sprite.setScale(scale);
		}
		if (ImGui::DragFloat("scale y", &scale.y))
		{
			sprite.setScale(scale);
		}

		Vec2f origin = sprite.getOrigin();
		if (ImGui::DragFloat("origin x", &origin.x))
		{
			sprite.setOrigin(origin);
		}
		if (ImGui::DragFloat("origin y", &origin.y))
		{
			sprite.setOrigin(origin);
		}
	}

	else if (circle_obj != nullptr)
	{
		auto& circle = circle_obj->GetCircle();

		const sf::Texture* attached_texture = circle.getTexture();

		ImGui::Text("texture");
		ImGui::SameLine();

		if (attached_texture != nullptr)
		{
			if (ImGui::ImageButton("texturebutton", *attached_texture, { 25, 25 }))
			{
				// TODO:
			}
		}
		else
		{
			if (ImGui::Button("##notex", { 25, 25 }))
			{

			}
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("move file");
			if (payload != nullptr)
			{
				std::filesystem::path src = *(std::string*)payload->Data;
				do
				{
					if (!src.has_extension() || (src.extension().string() != ".jpg" && src.extension().string() != ".png"))
					{
						break;
					}

					std::filesystem::path relative = std::filesystem::relative(src, asset_browser.GetAssetPath());
					if (ResourceManager::GetTextures().GetData().contains(relative.string()))
					{
						sf::Texture* managed_texture = ResourceManager::GetTextures().Get(relative.string());
						circle_obj->SetTexture(relative, managed_texture);
					}
					else
					{
						sf::Texture new_texture;
						if (!new_texture.loadFromFile(src.string()))
						{
							LOGERRORF("Failed to load texture from {}", src);
						}
						sf::Texture* managed_texture = ResourceManager::GetTextures().Add(relative.string(), new_texture);
						if (managed_texture != nullptr)
						{
							circle_obj->SetTexture(relative, managed_texture);
						}
					}

				} while (false);
			}
			ImGui::EndDragDropTarget();
		}

		if (attached_texture != nullptr)
		{
			if (ImGui::TreeNode("texture rect"))
			{
				sf::IntRect rect = circle.getTextureRect();
				if (ImGui::DragIntRect("circle rect", &rect))
				{
					circle.setTextureRect(rect);
				}
		

				ImGui::TreePop();
			}
		}

		const auto& circle_col = circle.getFillColor();
		float col[4] = {
			circle_col.r / 255.f,
			circle_col.g / 255.f,
			circle_col.b / 255.f,
			circle_col.a / 255.f
		};

		if (ImGui::ColorEdit4("fill color", col))
			circle.setFillColor(sf::Color{
			(uint8_t)(col[0] * 255.f),
			(uint8_t)(col[1] * 255.f),
			(uint8_t)(col[2] * 255.f),
			(uint8_t)(col[3] * 255.f) });

		auto circle_radius = circle.getRadius();
		if (ImGui::DragFloat("radius", &circle_radius))
			circle.setRadius(circle_radius);

		Vec2f scale = circle.getScale();
		if (ImGui::DragFloat("scale x", &scale.x))
		{
			circle.setScale(scale);
		}
		if (ImGui::DragFloat("scale y", &scale.y))
		{
			circle.setScale(scale);
		}
	}

	else if (audio_obj != nullptr)
	{
		float volume = audio_obj->GetVolume();
		float pitch = audio_obj->GetPitch();
		Vec3f spatial_pos = audio_obj->GetAudioPosition();
		const AudioSource* audio_source = audio_obj->GetAudioSource();

		if (ImGui::SliderFloat("volume", &volume, 0, 100))
		{
			audio_obj->SetVolume(volume);
		}
		if (ImGui::SliderFloat("pitch", &pitch, 0, 2))
		{
			audio_obj->SetPitch(pitch);
		}

		ImGui::BeginDisabled(audio_obj->GetChannels() > 1 || audio_obj->GetChannels() == 0);
		if (
			ImGui::DragFloat("spatial x", &spatial_pos.x) ||
			ImGui::DragFloat("spatial y", &spatial_pos.y) ||
			ImGui::DragFloat("spatial z", &spatial_pos.z))
		{
			audio_obj->SetAudioPosition(spatial_pos);
		}
		ImGui::EndDisabled();

		ImGui::Text("source file");
		ImGui::SameLine();

		if (audio_source != nullptr && !audio_source->full_path.empty())
		{
			if (ImGui::Button(audio_obj->GetAudioSource()->relative_path.string().c_str(), { 25, 25 }))
			{
				// TODO:
			}
		}
		else
		{
			if (ImGui::Button("##notex", { 25, 25 }))
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
					if (!src.has_extension() || (src.extension() != ".mp3" && src.extension() != ".wav" && src.extension() != ".ogg"))
					{
						break;
					}

					std::filesystem::path relative = std::filesystem::relative(src, asset_browser.GetAssetPath());
					AudioSource* managed_sound_buffer = ResourceManager::GetAudioSources().Get(relative.string());

					if (managed_sound_buffer != nullptr)
					{
						audio_obj->SetSource(managed_sound_buffer);
					}
					else
					{
						AudioSource new_src;
						new_src.full_path = src;
						new_src.relative_path = relative;

						if (!audio_obj->PlaysFromSource())
						{
							sf::SoundBuffer sound_buffer;
							if (!sound_buffer.loadFromFile(src.string()))
							{
								LOGERRORF("Failed to soundbuffer from {}", src);
							}

							new_src.sound_buffer = sound_buffer;
							new_src.has_valid_buffer = true;
						}
						else
						{
							new_src.has_valid_buffer = false;
						}

						AudioSource* managed_audio_source = ResourceManager::GetAudioSources().Add(relative.string(), new_src);
						if (managed_audio_source != nullptr)
						{
							audio_obj->SetSource(managed_audio_source);
						}
					}

				} while (false);
			}

			ImGui::EndDragDropTarget();
		}

		if (audio_source != nullptr && !audio_source->full_path.empty())
		{
			bool play_from_src = !audio_obj->PlaysFromSource();
			if (ImGui::Checkbox("cache source", &play_from_src))
			{
				audio_obj->ShouldPlayFromSource(!play_from_src);
			}

			if (ImGui::Button("Play"))
			{
				audio_obj->Play();
			}
			ImGui::SameLine();
			if (ImGui::Button("Pause"))
			{
				audio_obj->Pause();
			}
			ImGui::SameLine();
			if (ImGui::Button("Stop"))
			{
				audio_obj->Stop();
			}

			float time_line = audio_obj->GetTimeLine().asSeconds();
			if (ImGui::SliderFloat("time", &time_line, 0, audio_obj->GetDuration().asSeconds()))
			{
				audio_obj->SetTimeLine(time_line);
			}
		}
	}

	else if (text_obj != nullptr)
	{
		char buf[512];
		strncpy(buf, text_obj->GetText().c_str(), text_obj->GetText().length() + 1);
		if (ImGui::InputTextMultiline("Text", buf, sizeof(buf), {}))
		{
			text_obj->SetText(buf);
		}

		if (ImGui::TreeNode("Style properties"))
		{
			sf::Text& text = text_obj->GetTextObj();
			Toad::TextStyle text_style = text_obj->GetStyle();
			
			if (ImGui::Button("set default font"))
			{
				sf::Font* font = ResourceManager::GetFonts().Get("Default");
				
				// check if we don't already have a default font
				if (font == nullptr)
				{
					sf::Font new_default_font = GetDefaultFont();
				
					ResourceManager::GetFonts().Add("Default", new_default_font);
					text_obj->SetFont("Default", new_default_font);
				}
				else
				{
					text_obj->SetFont("Default", *font);
				}
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("move file"))
				{
					std::filesystem::path src = *(std::string*)payload->Data;
					do
					{
						if (!src.has_extension() || (
							src.extension().string() != ".ttf" &&
							src.extension().string() != ".pfa" &&
							src.extension().string() != ".pfb" &&
							src.extension().string() != ".cff" &&
							src.extension().string() != ".otf" &&
							src.extension().string() != ".sfnt" &&
							src.extension().string() != ".pcf" &&
							src.extension().string() != ".fnt" &&
							src.extension().string() != ".bdf" &&
							src.extension().string() != ".pfr" &&
							src.extension().string() != ".t42"
							))
						{
							break;
						}

						std::filesystem::path relative = std::filesystem::relative(src, asset_browser.GetAssetPath());
						if (ResourceManager::GetFonts().GetData().contains(relative.string()))
						{
							sf::Font* managed_font = ResourceManager::GetFonts().Get(relative.string());
							text_obj->SetFont(relative, *managed_font);
						}
						else
						{
							sf::Font new_font;
							if (!new_font.openFromFile(src.string()))
							{
								LOGERRORF("Failed to load font from {}", src);
							}
							sf::Font* managed_font = ResourceManager::GetFonts().Add(relative.string(), new_font);
							if (managed_font != nullptr)
							{
								text_obj->SetFont(relative, *managed_font);
							}
						}

					} while (false);
				}

				ImGui::EndDragDropTarget();
			}

			float fill_col[4] = {
				text_style.fill_col.r / 255.f,
				text_style.fill_col.g / 255.f,
				text_style.fill_col.b / 255.f,
				text_style.fill_col.a / 255.f
			};

			float outline_col[4] = {
				text_style.outline_col.r / 255.f,
				text_style.outline_col.g / 255.f,
				text_style.outline_col.b / 255.f,
				text_style.outline_col.a / 255.f
			};

			if (ImGui::ColorEdit4("fill color", fill_col))
			{
				sf::Color col = sf::Color(
					static_cast<uint8_t>(fill_col[0] * 255.f),
					static_cast<uint8_t>(fill_col[1] * 255.f),
					static_cast<uint8_t>(fill_col[2] * 255.f),
					static_cast<uint8_t>(fill_col[3] * 255.f)
				);

				text.setFillColor(col);

				text_style.fill_col = col;
				text_obj->SetStyle(text_style, false);
			}

			if (ImGui::ColorEdit4("outline color", outline_col))
			{
				sf::Color col = sf::Color(
					static_cast<uint8_t>(outline_col[0] * 255.f),
					static_cast<uint8_t>(outline_col[1] * 255.f),
					static_cast<uint8_t>(outline_col[2] * 255.f),
					static_cast<uint8_t>(outline_col[3] * 255.f)
				);

				text.setOutlineColor(col);

				text_style.outline_col = col;
				text_obj->SetStyle(text_style, false);
			}
			int char_size_int = (int)text_style.char_size;
			if (ImGui::InputInt("character size", &char_size_int, 1, 3))
			{
				char_size_int = std::clamp(char_size_int, 0, INT_MAX);
				text_style.char_size = char_size_int;
				text.setCharacterSize(text_style.char_size);
				text_obj->SetStyle(text_style, false);
			}
			if (ImGui::InputFloat("character spacing", &text_style.char_spacing, 1, 3))
			{
				text.setLetterSpacing(text_style.char_spacing);
				text_obj->SetStyle(text_style, false);
			}
			if (ImGui::InputFloat("line spacing", &text_style.line_spacing, 1, 3))
			{
				text.setLineSpacing(text_style.line_spacing);
				text_obj->SetStyle(text_style, false);
			}
			if (ImGui::InputFloat("outline thickness", &text_style.outline_thickness, 1, 3))
			{
				text.setOutlineThickness(text_style.outline_thickness);
				text_obj->SetStyle(text_style, false);
			}

			if (ImGui::BeginCombo("style", ""))
			{
				if (ImGui::Selectable("Regular", text_style.style & sf::Text::Style::Regular))
				{
					text_style.style = static_cast<sf::Text::Style>(static_cast<int>(text_style.style) ^ static_cast<int>(sf::Text::Style::Regular));
					text_obj->SetStyle(text_style);
				}
				if (ImGui::Selectable("Bold", text_style.style & sf::Text::Style::Bold))
				{
					text_style.style = static_cast<sf::Text::Style>(static_cast<int>(text_style.style) ^ static_cast<int>(sf::Text::Style::Bold));
					text_obj->SetStyle(text_style);
				}
				if (ImGui::Selectable("Italic", text_style.style & sf::Text::Style::Italic))
				{
					text_style.style = static_cast<sf::Text::Style>(static_cast<int>(text_style.style) ^ static_cast<int>(sf::Text::Style::Italic));
					text_obj->SetStyle(text_style);
				}
				if (ImGui::Selectable("Underlined", text_style.style & sf::Text::Style::Underlined))
				{
					text_style.style = static_cast<sf::Text::Style>(static_cast<int>(text_style.style) ^ static_cast<int>(sf::Text::Style::Underlined));
					text_obj->SetStyle(text_style);
				}
				if (ImGui::Selectable("StrikeThrough", text_style.style & sf::Text::Style::StrikeThrough))
				{
					text_style.style = static_cast<sf::Text::Style>(static_cast<int>(text_style.style) ^ static_cast<int>(sf::Text::Style::StrikeThrough));
					text_obj->SetStyle(text_style);
				}

				ImGui::EndCombo();
			}


			ImGui::TreePop();
		}
	}

	else if (cam_obj != nullptr)
	{
		bool active = cam_obj->IsActive();
		if (ImGui::Checkbox("Active", &active))
		{
			if (active)
			{
				cam_obj->ActivateCamera();
			}
			else
			{
				cam_obj->DeactivateCamera();
			}
		}

		Vec2f size = cam_obj->GetSize();
		if (ImGui::DragFloat("size x", &size.x))
		{
			cam_obj->SetSize(size);
		}

		if (ImGui::DragFloat("size y", &size.y))
		{
			cam_obj->SetSize(size);
		}
	}

	static std::string selected_script_name;

	bool script_node_open = ImGui::TreeNode("Scripts");
	if (ImGui::IsMouseReleased(1) && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
	{
		ImGui::OpenPopup("SCRIPT ADD");
	}
	if (ImGui::BeginPopup("SCRIPT ADD"))
	{
		for (auto& [name, script] : Toad::GetGameScriptsRegister())
		{
			if (!script)
				continue;

			if (ImGui::Button(name.c_str()))
			{
				selected_obj->AddScript(script->Clone());
				selected_obj->GetScript(name)->ExposeVars();
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
				ImGui::TextColored({ 0.2f,0.8f,0.2f,1.f }, "%p", script);

				auto script_vars = script->GetReflection().Get();
				const char* no_var_found_msg = "This variable is not found/exposed in this script, Resave the scene or re-expose the variable";
				for (auto& [name, var] : script_vars.str)
				{
					if (var)
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
					else
					{
						ImGui::TextColored({ 1.f,0.f,0.f,1.f }, "%s is null", name.c_str());
						ImGui::SameLine();
						HelpMarker(no_var_found_msg);
					}
				}
				for (auto& [name, var] : script_vars.b)
				{
					if (var)
					{
						ImGui::Checkbox(name.c_str(), var);
						ImGui::SameLine();
						ImGui::TextColored({ 0.2f,0.2f,0.2f,1.f }, "%p", var);
					}
					else
					{
						ImGui::TextColored({ 1.f,0.f,0.f,1.f }, "%s is null", name.c_str());
						ImGui::SameLine();
						HelpMarker(no_var_found_msg);
					}
				}
				for (auto& [name, var] : script_vars.flt)
				{
					if (var)
					{
						ImGui::DragFloat(name.c_str(), var);
						ImGui::SameLine();
						ImGui::TextColored({ 0.2f,0.2f,0.2f,1.f }, "%p", var);
					}
					else
					{
						ImGui::TextColored({ 1.f,0.f,0.f,1.f }, "%s is null", name.c_str());
						ImGui::SameLine();
						HelpMarker(no_var_found_msg);
					}

				}
				for (auto& [name, var] : script_vars.i8)
				{
					if (var)
					{
						ImGui::DragInt(name.c_str(), (int*)var, 1.0f, i8_min, i8_max);
						ImGui::SameLine();
						ImGui::TextColored({ 0.2f,0.2f,0.2f,1.f }, "%p", var);
					}
					else
					{
						ImGui::TextColored({ 1.f,0.f,0.f,1.f }, "%s is null", name.c_str());
						ImGui::SameLine();
						HelpMarker(no_var_found_msg);
					}
				}
				for (auto& [name, var] : script_vars.i16)
				{
					if (var)
					{
						ImGui::DragInt(name.c_str(), (int*)var, 1.0f, i16_min, i16_max);
						ImGui::SameLine();
						ImGui::TextColored({ 0.2f,0.2f,0.2f,1.f }, "%p", var);
					}
					else
					{
						ImGui::TextColored({ 1.f,0.f,0.f,1.f }, "%s is null", name.c_str());
						ImGui::SameLine();
						HelpMarker(no_var_found_msg);
					}
				}
				for (auto& [name, var] : script_vars.i32)
				{
					if (var)
					{
						ImGui::DragInt(name.c_str(), var);
						ImGui::SameLine();
						ImGui::TextColored({ 0.2f,0.2f,0.2f,1.f }, "%p", var);
					}
					else
					{
						ImGui::TextColored({ 1.f,0.f,0.f,1.f }, "%s is null", name.c_str());
						ImGui::SameLine();
						HelpMarker(no_var_found_msg);
					}
				}

				ImGui::SeparatorText("OnEditorUI");

				static bool once = false;

				script->OnEditorUI(selected_obj, ImGui::GetCurrentContext());

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
			if (ImGui::Button("SCRIPT DELETE"))
			{
				selected_obj->RemoveScript(selected_script_name);
			}
			if (ImGui::Button("SCRIPT COPY"))
			{
				// selected_obj->SerializeScripts()
			}
			ImGui::EndPopup();
		}

		ImGui::TreePop();
	}
}

}
