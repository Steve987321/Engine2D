#include "pch.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "AnimationEditor.h"

#include "engine/Engine.h"

#include "UI.h"

#include "imgui-SFML.h"
#include "utils/FileDialog.h"

namespace Toad
{

	AnimationEditor::AnimationEditor()
		: m_spriteSheet(*ResourceManager::GetTextures().Get("Default"))
	{
		m_inspectorUI = std::bind(&AnimationEditor::ShowAnimationPropsUI, this);

		m_cam.SetSize({1920, 1080});
		m_cam.original_size = m_cam.GetSize();
	}

	AnimationEditor::~AnimationEditor() = default;

	void AnimationEditor::Show(bool* show)
	{
		static Vec2i starting_position{ 0,0 };
		static Vec2i spacing{ 100,100 };
		static Vec2i tile_size{ 100,100 };

		bool animation_editor_opened = ImGui::Begin("AnimationEditor", show);

		if (animation_editor_opened)
			 SetInteractingTexture(&m_previewTexture);

		{
			std::string info_title = format_str("{} | (W:{}H:{})", m_selectedAnimation.name + FILE_EXT_TOADANIMATION, m_previewTexture.getSize().x, m_previewTexture.getSize().y);
			ImGui::TextColored({ 1, 1, 1, 0.7f }, "%s", info_title.c_str());

			// #TODO: add sequence converter (to tilesheet) and finish this behavior
			ImGui::BeginDisabled();
			if (ImGui::Button("Load image sequence"))
			{
				for (const auto& s : m_textureIds)
				{
					ResourceManager::GetTextures().Remove(s);
				}
				m_textureIds.clear();

				std::vector<std::string> files = GetPathFiles(get_exe_path().string(), "PNG or JPG (*.png;*.jpg)\0*.png;*.jpg\0 PNG (*.png)\0*.png\0 JPG (*.jpg)\0*.jpg\0");
				if (!files.empty())
				{
					const std::string& dir = files[0];
					for (size_t i = 0; i < files.size(); i++)
					{
#ifdef _WIN32
						if (i == 0)
							continue;
						std::string path = dir + PATH_SEPARATOR + files[i];
#else
						std::string path = files[i];
#endif
						LOGDEBUGF("[AnimationEditor] Loading texture from: {}", path);

						sf::Texture* managed_texture = ResourceManager::GetTextures().Get(path);

						Vec2f pos {managed_texture->getSize().x * i / 2.f, 50};
						if (managed_texture)
						{
							sf::Sprite s(*managed_texture);
							s.setPosition(pos);
							m_sequence.emplace_back(s);
							m_textureIds.emplace_back(path);
						}
						else
						{
							sf::Texture t;
							if (t.loadFromFile(path))
							{
								managed_texture = ResourceManager::GetTextures().Add(path, t);
								sf::Sprite s(*managed_texture);
								s.setPosition(pos);
								m_sequence.emplace_back(s);
								m_textureIds.emplace_back(path);
							}
							else
								LOGERRORF("[AnimationEditor] Failed to load: {}", path);
						}
					}
				}
			}
			ImGui::EndDisabled();

			ImGui::SameLine();
			if (ImGui::Button("Load tilesheet"))
			{
				for (const auto& s : m_textureIds)
				{
					ResourceManager::GetTextures().Remove(s);
				}
				m_sequence.clear();
				m_textureIds.clear();

				//
				// #TODO: Drag and drop from game assets, don't allow files from other directories other then game assets directory 
				//

				std::string file = GetPathFile(get_exe_path().string(), "PNG (*.png)\0*.png\0 JPG (*.jpg)\0*.jpg\0");
				sf::Texture* managed_texture = ResourceManager::GetTextures().Get(file);
				if (managed_texture)
				{
					m_spriteSheet = sf::Sprite(*managed_texture);
					m_textureIds.emplace_back(file);

					ImGui::OpenPopup("Tile detector");
				}
				else
				{
					sf::Texture t;
					if (t.loadFromFile(file))
					{
						managed_texture = ResourceManager::GetTextures().Add(file, t);
						m_spriteSheet = sf::Sprite(*managed_texture);

						Vec2u tex_size = m_spriteSheet.getTexture().getSize();
						bool resize_success = m_previewTexture.resize(tex_size);
						assert(resize_success && "failed to resize previewtexture in animation editor");

						m_textureIds.emplace_back(file);
					}
					else
						LOGERRORF("[AnimationEditor] Failed to load: {}", file);
				}
			}

			if (ImGui::Button("Change texture resolution"))
			{
				ImGui::OpenPopup("ChangeTexResPopup");
			}
			if (ImGui::BeginPopup("ChangeTexResPopup", ImGuiWindowFlags_AlwaysAutoResize))
			{
				static Vec2i size = { (int)m_previewTexture.getSize().x, (int)m_previewTexture.getSize().y };
				ImGui::SliderVec2i("size", &size, 1);
				if (ImGui::Button("ok"))
				{
					bool resize_success = m_previewTexture.resize({(uint32_t)size.x, (uint32_t)size.y});
					assert(resize_success && "failed to resize texture");
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Button("cancel"))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			
			ImGui::SliderVec2i("start", &starting_position);
			ImGui::SliderVec2i("spacing", &spacing);
			ImGui::SliderVec2i("tile size", &tile_size);

			// in a scope so it can be collapsed
			{

				//if (ImGui::BeginPopup("Tile detector"))
				//{
				//	static sf::Color bg_color(255, 255, 255, 0);
				//	static float bg_color_f4[4] = { 1, 1, 1, 0 };

				//	if (ImGui::ColorEdit4("fill color", bg_color_f4))
				//	{
				//		bg_color = sf::Color(
				//			static_cast<uint8_t>(bg_color_f4[0] * 255.f),
				//			static_cast<uint8_t>(bg_color_f4[1] * 255.f),
				//			static_cast<uint8_t>(bg_color_f4[2] * 255.f),
				//			static_cast<uint8_t>(bg_color_f4[3] * 255.f)
				//		);
				//	}

				//	if (ImGui::Button("Begin"))
				//	{
				//		sf::Image image = m_spriteSheet.getTexture()->copyToImage();

				//		Vec2u tile_size{ 0, 0 };

				//		for (uint32_t x = 0; x < image.getSize().x; x++)
				//		{
				//			for (uint32_t y = 0; y < image.getSize().y; y++)
				//			{
				//				sf::Color col = image.getPixel(x, y);

				//				if (col == bg_color)
				//					continue;

				//				Vec2u pos(x, y);

				//				//tile_positions.emplace_back();

				//				//LOGDEBUGF("{} {}, COL(R:{}, G:{}, B:{}, A:{})", x, y, (int)col.r, (int)col.g, (int)col.b, (int)col.a);
				//			}
				//		}

				//		ImGui::CloseCurrentPopup();
				//	}

				//	ImGui::EndPopup();
				//}
			}

			m_previewTexture.setView(m_cam.GetView());
			m_previewTexture.clear(sf::Color::Black);

			if (!m_sequence.empty())
				ImGui::TextColored({ 0.7f, 0.7f, 0.7f, 0.6f }, "sequence animations should first be converted to a spritesheet");
			else
				m_previewTexture.draw(m_spriteSheet);

			m_previewTexture.display();

			ImVec2 preview_texture_scursor_pos;

			ImGui::BeginChild("PreviewScreen", { 0,0 }, true);
			{
				preview_texture_scursor_pos = ImGui::GetCursorScreenPos();
				ImVec2 content_size = ImGui::GetContentRegionAvail();
				ImGui::Image(m_previewTexture, {content_size.x, content_size.y}, sf::Color::White);

				ImDrawList* draw = ImGui::GetWindowDrawList();

				for (uint32_t x = (uint32_t)starting_position.x; x < m_previewTexture.getSize().x;)
				{
					for (uint32_t y = (uint32_t)starting_position.y; y < m_previewTexture.getSize().y;)
					{
						Vec2f screen_start = Screen::WorldToScreen({ (float)x, (float)y }, m_cam, { content_size.x, content_size.y }, { preview_texture_scursor_pos.x, preview_texture_scursor_pos.y });
						Vec2f screen_end = Screen::WorldToScreen({ (float)x + tile_size.x, (float)y + tile_size.y }, m_cam, { content_size.x, content_size.y }, { preview_texture_scursor_pos.x, preview_texture_scursor_pos.y });
						draw->AddRect({screen_start.x, screen_start.y}, { screen_end.x, screen_end.y }, IM_COL32_WHITE);
						y += tile_size.y + spacing.y;
					}
					x += tile_size.x + spacing.x;
				}
			}
			ImGui::SameLine();
			ImGui::Begin("PreviewOutputAnimation");
			{
				ImVec2 cursor_pos_begin = ImGui::GetCursorPos();
				ImGui::Image(m_previewOutputAnimationTexture, { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y });

				ImGui::SetCursorPos(cursor_pos_begin);
				if (ImGui::TreeNode("Options"))
				{

					ImGui::TreePop();
				}

			}
			ImGui::End();
			ImGui::EndChild();

			if (animation_editor_opened)
			{
				Vec2u size = m_previewTexture.getSize();
				ImDrawList* draw = ImGui::GetForegroundDrawList();
				constexpr ImU32 border_col = IM_COL32(255, 255, 255, 250);

				if (ImGui::IsKeyDown(ImGuiKey_ModSuper))
				{
					static bool previously_dragging = false;
					static bool capture_first_click = true;
					ImVec2 start{};
					if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && capture_first_click)
					{
						start = ImGui::GetMousePos();
						capture_first_click = false;
					}
					else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
						capture_first_click = true;

					if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
					{
						previously_dragging = true;
					}
					else
					{
						ImVec2 end = start + ImGui::GetMousePos();
						Vec2f world_start = Screen::ScreenToWorld({ (int)start.x, (int)start.y }, m_cam);
						Vec2f world_dist = Screen::ScreenToWorld({ (int)end.x, (int)end.y }, m_cam) - world_start;

						starting_position = {(int)world_start.x, (int)world_start.y};
						tile_size = {(int)world_start.x, (int)world_start.y};
						
						if (previously_dragging)
							previously_dragging = false;
					}
				}

				// movement 
				if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
				{
					ImGuiContext* g = ImGui::GetCurrentContext();
					ImVec2 d = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle, 0.f);
					g->IO.MouseClickedPos[ImGuiMouseButton_Middle] = ImGui::GetMousePos();
					m_cam.SetPosition(m_cam.GetPosition() - Vec2f(d.x, d.y));
				}

				float mwheel = ImGui::GetIO().MouseWheel;
				if (mwheel)
				{
					if (mwheel < 0)
					{
						m_cam.Zoom(1.2f);
					}
					else
					{
						m_cam.Zoom(0.8f);
					}
				}
			}
		}
		ImGui::End();

		{
			//ImGui::Begin("Timeline");
			//{
			//	if (ImGui::IsWindowFocused())
			//		ui::inspector_ui = m_inspectorUI;

			//	static int current_frame = 0;

			//	ImGui::BeginChild("TimelineMenuBar", { 0, 50 }, true);
			//	{
			//		ImGui::PushItemWidth(30);
			//		if (ImGui::Button("<"))
			//			current_frame--;
			//		ImGui::SameLine();
			//		if (ImGui::Button(">"))
			//			current_frame++;

			//		ImGui::Separator();

			//		ImGui::PopItemWidth();
			//	}
			//	ImGui::EndChild();

			//	ImGui::End();
			//}
		}

	}
	void AnimationEditor::LoadAnimationFromSequence()
	{

	}

	void AnimationEditor::LoadAnimationFromTileSheet(const fs::path& path)
	{

	}

	void AnimationEditor::ShowAnimationPropsUI()
	{
		ImGui::Text("%s", (m_selectedAnimation.name + FILE_EXT_TOADANIMATION).c_str());

		if (!m_sequence.empty())
		{
			ImGui::Text("sequence");
			static int selected = -1;
			for (int i = 0; i < m_sequence.size(); i++)
			{
				if (ImGui::Selectable(std::to_string(i).c_str(), selected == i))
				{
					selected = i;
				}
			}

			if (selected < m_sequence.size() && selected >= 0)
			{
				auto& sprite = m_sequence[selected];
				ImGui::SeparatorText("properties");
				Vec2f pos = sprite.getPosition();
				Vec2f scale = sprite.getScale();
				if (ImGui::SliderVec2("position", &pos))
				{
					sprite.setPosition(pos);
				}
				if (ImGui::SliderVec2("scale", &scale))
				{
					sprite.setScale(scale);
				}

			}
		}
		else
		{

		}
	}

}
