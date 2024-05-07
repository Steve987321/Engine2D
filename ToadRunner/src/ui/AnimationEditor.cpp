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
		ui::inspector_ui = m_inspectorUI;

		ImGui::Begin("AnimationEditor", show);
		{
			std::string info_title = format_str("{} | (W:{}H:{})", m_selectedAnimation.name + FILE_EXT_TOADANIMATION, m_previewTexture.getSize().x, m_previewTexture.getSize().y);
			ImGui::TextColored({ 1, 1, 1, 0.7f }, info_title.c_str());

			ResourceManager& resource_manager = Engine::Get().GetResourceManager();

			// #TODO: add sequence converter (to tilesheet) and finish this behavior
			if (ImGui::Button("LOAD SEQUENCE"))
			{
				for (const auto& s : m_textureIds)
				{
					resource_manager.RemoveTexture(s);
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

						sf::Texture* managed_texture = resource_manager.GetTexture(path);

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
								managed_texture = resource_manager.AddTexture(path, t);
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
			ImGui::SameLine();
			if (ImGui::Button("LOAD TILESHEET"))
			{
				for (const auto& s : m_textureIds)
				{
					resource_manager.RemoveTexture(s);
				}
				m_sequence.clear();
				m_textureIds.clear();

				std::string file = GetPathFile(get_exe_path().string(), "PNG (*.png)\0*.png\0 JPG (*.jpg)\0*.jpg\0");
				sf::Texture* managed_texture = resource_manager.GetTexture(file);
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
						managed_texture = resource_manager.AddTexture(file, t);
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
		ImGui::End();

		ImGui::Begin("Timeline");
		{
			ImGui::BeginChild("TimelineMenuBar", { 0, 0 }, true);
			ImGui::EndChild();
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