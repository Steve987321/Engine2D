#include "pch.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "AnimationEditor.h"

#include "Engine/Engine.h"

#include "UI.h"

#include "imgui-SFML.h"
#include "utils/FileDialog.h"

namespace Toad
{

	AnimationEditor::AnimationEditor()
	{
		m_inspectorUI = std::bind(&AnimationEditor::ShowAnimationPropsUI, this);
		m_previewTexture.create(1920, 1080);
	}

	AnimationEditor::~AnimationEditor() = default;

	void AnimationEditor::Show(bool* show)
	{
		bool animation_editor_opened = ImGui::Begin("AnimationEditor", show);
		{
			std::string info_title = format_str("{} | (W:{}H:{})", m_selectedAnimation.name + FILE_EXT_TOADANIMATION, m_previewTexture.getSize().x, m_previewTexture.getSize().y);
			ImGui::TextColored({ 1, 1, 1, 0.7f }, info_title.c_str());

			// #TODO: add sequence converter (to tilesheet) and finish this behavior
			ImGui::BeginDisabled();
			if (ImGui::Button("LOAD SEQUENCE"))
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

						if (managed_texture)
						{
							sf::Sprite s(*managed_texture);
							s.setPosition(managed_texture->getSize().x * i / 2.f, 50);
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
								s.setPosition(managed_texture->getSize().x * i / 2.f, 50);
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
			if (ImGui::Button("LOAD TILESHEET"))
			{
				for (const auto& s : m_textureIds)
				{
					ResourceManager::GetTextures().Remove(s);
				}
				m_sequence.clear();
				m_textureIds.clear();

				std::string file = GetPathFile(get_exe_path().string(), "PNG (*.png)\0*.png\0 JPG (*.jpg)\0*.jpg\0");
				sf::Texture* managed_texture = ResourceManager::GetTextures().Get(file);
				if (managed_texture)
				{
					m_spriteSheet = sf::Sprite(*managed_texture);
					m_textureIds.emplace_back(file);
				}
				else
				{
					sf::Texture t;
					if (t.loadFromFile(file))
					{
						managed_texture = ResourceManager::GetTextures().Add(file, t);
						m_spriteSheet = sf::Sprite(*managed_texture);
						m_textureIds.emplace_back(file);
					}
					else
						LOGERRORF("[AnimationEditor] Failed to load: {}", file);
				}
			}

			if (ImGui::Button("CHANGE TEXTURE RES"))
			{
				ImGui::OpenPopup("ChangeTexResPopup");
			}
			if (ImGui::BeginPopup("ChangeTexResPopup", ImGuiWindowFlags_AlwaysAutoResize))
			{
				static Vec2i size = {(int)m_previewTexture.getSize().x, (int)m_previewTexture.getSize().y};
				ImGui::SliderVec2i("size", &size, 1);
				if (ImGui::Button("ok"))
				{
					m_previewTexture.create(size.x, size.y);
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Button("cancel"))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			m_previewTexture.setView(m_cam.GetView());

			m_previewTexture.clear(sf::Color::Black);

			for (const auto & s : m_sequence)
			{
				m_previewTexture.draw(s);
			}

			if (!m_sequence.empty())
				ImGui::TextColored({ 0.7f, 0.7f, 0.7f, 0.6f }, "sequence animations should first be converted to a spritesheet");
			else
				m_previewTexture.draw(m_spriteSheet);

			m_previewTexture.display();

			ImGui::BeginChild("PreviewScreen", { 0,0 }, true);
			{
				ImGui::Image(m_previewTexture, { ImGui::GetContentRegionAvail().x,  ImGui::GetContentRegionAvail().y }, sf::Color::White);
			}
			ImGui::SameLine();
			ImGui::Begin("PreviewOutputAnimation");
			{
				ImGui::Image(m_previewOutputAnimationTexture, { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y });
				ImGui::End();
			}
			ImGui::EndChild();

			if (animation_editor_opened)
			{
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

		ImGui::Begin("Timeline");
		{
			if (ImGui::IsWindowFocused())
				ui::inspector_ui = m_inspectorUI;

			static int current_frame = 0;

			ImGui::BeginChild("TimelineMenuBar", { 0, 50 }, true);
			{
				ImGui::PushItemWidth(30);

				if (ImGui::DragInt("pos", &current_frame))
					current_frame = std::clamp(current_frame, 0, m_selectedAnimation.frame_length);

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					if (ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()))
					{
						ImGui::BeginPopup("AddObjectToTimeLine");
					}
				}

				ImGui::PopItemWidth();
			}
			ImGui::EndChild();

			if (ImGui::BeginPopup("AddObjectToTimeLine"))
			{
				if (ImGui::BeginMenu("Create"))
				{
					if (ImGui::MenuItem("Circle"))
					{
						m_selectedAnimation.frames[0].is_key = true;
					}
					if (ImGui::MenuItem("Sprite"))
					{
					}
					if (ImGui::MenuItem("Audio"))
					{
					}
					if (ImGui::MenuItem("Text"))
					{
					}
					if (ImGui::MenuItem("Camera"))
					{
					}

					ImGui::EndMenu();
				}

				ImGui::EndPopup();
			}

			ImGui::BeginChild("TimelineList", { 50, 0 }, true);
			{
				
			}
			ImGui::EndChild();

			auto draw = ImGui::GetWindowDrawList();
			auto current_pos = ImGui::GetCursorPos();

			for (size_t i = 0; i < m_selectedAnimation.frames.size(); i++)
			{
				float pos_x = current_pos.x + (float)i * 10.f;
				//if (pos_x > current_pos.x + ImGui::GetContentRegionAvail().x) // #TODO: fix check
				//	break;

				draw->AddCircleFilled(ImGui::GetCursorScreenPos() + ImVec2{ pos_x, current_pos.y }, 2.f, IM_COL32_WHITE);
			}

			// timeline dragger 
			ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2{ current_pos.x + current_frame * 10.f - 1, current_pos.y + 5};

			static ImU32 timeline_drag_col = ImGui::ColorConvertFloat4ToU32({ 1, 1, 1, 0.4f });
			// dragger
			const int triangle_size = 5;
			ImRect triangle_rect(pos + ImVec2{ -triangle_size, -triangle_size }, pos + ImVec2{ triangle_size, triangle_size });
			triangle_rect.Expand(5);
			draw->AddTriangleFilled(pos + ImVec2{ -triangle_size + 1, -triangle_size + 1}, pos + ImVec2{ triangle_size + 1, -triangle_size + 1 }, pos + ImVec2{1, triangle_size + 1}, timeline_drag_col);

			// line 
			ImRect time_cursor_rect(pos, pos + ImVec2{ 2, ImGui::GetContentRegionAvail().y } );
			draw->AddRectFilled(time_cursor_rect.Min, time_cursor_rect.Max, timeline_drag_col);

			static bool time_cursor_dragging = false;

			if (ImGui::IsMouseHoveringRect(triangle_rect.Min, triangle_rect.Max))
			{
				timeline_drag_col = ImGui::ColorConvertFloat4ToU32({ 1, 1, 1, 0.6f });
				if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
				{
					timeline_drag_col = ImGui::ColorConvertFloat4ToU32({ 1, 1, 1, 0.2f });
					time_cursor_dragging = true; 
				}
			}
			else if (!time_cursor_dragging)
			{
				timeline_drag_col = ImGui::ColorConvertFloat4ToU32({ 1, 1, 1, 0.4f });
			}

			if (time_cursor_dragging)
			{
				// #TODO: Change mouse cursor to vertical thing
				float drag_x = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 10.f).x;
				if (drag_x <= -10)
				{
					current_frame -= round(abs(drag_x) / 10.f);
					ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
				}
				else if (drag_x >= 10)
				{
					current_frame += round(drag_x / 10.f);
					ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
				}

				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					time_cursor_dragging = false;
					timeline_drag_col = ImGui::ColorConvertFloat4ToU32({ 1, 1, 1, 0.4f });
				}

				current_frame = std::clamp(current_frame, 0, m_selectedAnimation.frame_length);
			}
		}
		ImGui::End();
	}

	void AnimationEditor::LoadAnimationFromSequence()
	{

	}

	void AnimationEditor::LoadAnimationFromTileSheet(const fs::path& path)
	{

	}

	void AnimationEditor::ShowAnimationPropsUI()
	{
		ImGui::Text((m_selectedAnimation.name + FILE_EXT_TOADANIMATION).c_str());

		ImGui::DragInt("Animation Frames", &m_selectedAnimation.frame_length);
		if (m_selectedAnimation.frame_length != m_selectedAnimation.frames.size())
		{
			ImGui::TextColored({ 1, 1, 0, 1 }, "The animation frames in the array are %d", (int)m_selectedAnimation.frames.size());
			if (ImGui::Button("resize animation frame length"))
			{
				m_selectedAnimation.frames.resize(m_selectedAnimation.frame_length);
			}
		}

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