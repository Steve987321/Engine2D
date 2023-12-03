#include "pch.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "Engine/Engine.h"

#include "GameAssetsBrowser.h"

#include "project/misc.h"
#include "project/ToadProject.h"

namespace Toad
{

GameAssetsBrowser::GameAssetsBrowser(std::string_view asset_path)
{
	SetAssetPath(asset_path);
}

GameAssetsBrowser::~GameAssetsBrowser()
{
}

void list_dir_contents(const fs::path& path)
{
	for (const auto& entry : fs::directory_iterator(path))
	{
		if (entry.is_directory())
		{
			if (ImGui::TreeNode(entry.path().filename().string().c_str()))
			{
				list_dir_contents(entry);

				ImGui::TreePop();
			}
		}
		else
		{
			if (ImGui::Button(entry.path().filename().string().c_str()))
			{
				// isnpecdt
				// double click open 
			}
		}
	}
}

void GameAssetsBrowser::Show()
{
	if (!m_assets_path.empty())
		list_dir_contents(m_assets_path);

	ImGui::Begin("second");
	
	if (m_assets_path.empty())
	{
		ImGui::Text("nothing to show");
		ImGui::End();
		return;
	}

	if (m_current_path != m_assets_path && ImGui::ArrowButton("##back", ImGuiDir_Left))
	{
		m_current_path = m_current_path.parent_path();
	}
	if (ImGui::Button("Open .sln"))
	{
		auto editors = misc::FindEditors();
		if (editors.empty())
		{
			LOGERROR("No visual studio found to open .sln");
		}
		else
		{
			for (const auto& editor : editors)
			{
				if (editor.name.find("Visual Studio") != std::string::npos)
				{
					// get .sln file
					// TODO: temp
					auto temp = m_assets_path.parent_path().parent_path().parent_path();
					fs::path slnpath;
					for (const auto& entry : fs::recursive_directory_iterator(temp))
					{
						if (entry.is_regular_file())
						{
							if (entry.path().extension() == ".sln")
							{
								slnpath = entry;
								break;
							}
						}
					}

					if (slnpath.empty())
					{
						LOGERRORF("can't find .sln file in {}", temp);
						break;
					}

					if (!project::OpenSln(slnpath, editor))
					{
						LOGERRORF("Failed to open {} with {}", slnpath, editor.name);
					}
					
					break;
				}
			}
		}
	}

	static fs::path selected;
	static bool renaming = false;
	static char renaming_buf[100];

	// pop ups 
	if (ImGui::BeginPopup("creation menu"))
	{
		ImGui::SeparatorText("Add");

		if (ImGui::MenuItem("Directory"))
		{
			std::string dir_name = "new_directory";
			while (exists(m_current_path / dir_name))
			{
				dir_name += "_1";
			}
			create_directory(m_current_path / dir_name);

			selected = m_current_path / dir_name;
			strcpy_s(renaming_buf, selected.filename().string().c_str());
			renaming = true;

			ImGui::CloseCurrentPopup();
		}

		if (ImGui::MenuItem("Scene"))
		{
			std::string scene_name = "Scene.TSCENE";
			while (exists(m_current_path / scene_name))
			{
				scene_name += "_1";
			}

			std::ofstream f(m_current_path / scene_name);
			nlohmann::json da;
			f << da;
			f.close();

			selected = m_current_path / scene_name;
			strcpy_s(renaming_buf, selected.filename().string().c_str());
			renaming = true;

			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	else if (ImGui::BeginPopup("modify menu"))
	{
		ImGui::SeparatorText(selected.filename().string().c_str());

		// TODO: Copy Cut Paste & Shortcuts
		if (ImGui::MenuItem("Copy", "CTRL+C"))
		{

			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("Cut", "CTRL+X"))
		{

			ImGui::CloseCurrentPopup();
		}

		// Paste

		if (ImGui::MenuItem("Delete"))
		{
			fs::remove(selected);
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	int i = 0;
	for (const auto& entry: fs::directory_iterator(m_current_path))
	{
		i++;

		if (!selected.empty())
		{
			// rename
			if (!renaming && selected == entry.path() && ImGui::IsKeyPressed(ImGuiKey_F2))
			{
				strcpy_s(renaming_buf, entry.path().filename().string().c_str());
				renaming = true;
			}
		}

		if (entry.is_directory())
		{
			ImGui::PushID(i);
			if (ImGui::Selectable("D", selected == entry.path(), 0, { 50, 50 }))
			{
				selected = entry.path();
			}
			ImGui::PopID();

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			{
				m_current_path = entry.path();
			}
		}
		else
		{
			ImGui::PushID(i);
			if (ImGui::Selectable("F", selected == entry.path(), 0, {50, 50}))
			{
				selected = entry.path();
			}
			ImGui::PopID();
		}

		// options
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsItemHovered())
		{
			selected = entry.path();
			ImGui::OpenPopup("modify menu");
		}
		if (renaming && selected == entry.path())
		{
			ImGui::SetKeyboardFocusHere();

			if (ImGui::InputText("##", renaming_buf, 100, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				fs::rename(entry.path(), entry.path().parent_path() / renaming_buf);
				renaming = false;
			}
			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				strcpy_s(renaming_buf, entry.path().filename().string().c_str());
				renaming = false;
			}
		}
		else
		{
			ImGui::Text(entry.path().filename().string().c_str());
		}
	}

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		if (ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()))
		{
			if (!ImGui::IsPopupOpen("modify menu"))
				ImGui::OpenPopup("creation menu");
		}

	}

	ImGui::End();
}

void GameAssetsBrowser::SetAssetPath(std::string_view path)
{
	m_assets_path = path;
	m_current_path = path;
}

const fs::path& GameAssetsBrowser::GetAssetPath()
{
	return m_assets_path;
}

}
