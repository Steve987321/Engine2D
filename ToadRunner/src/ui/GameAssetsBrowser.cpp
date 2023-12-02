#include "pch.h"

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

	static bool creation_menu = false;
	static ImVec2 creation_menu_pos;

	// TODO: ENABLE WHEN RIGHT CLICK ON THIS RECT 
	if (creation_menu)
	{
		ImGui::SetCursorPos(creation_menu_pos);
		ImGui::BeginChild("Create", {}, true);

		if (ImGui::Button("Directory", { 50, 0 }))
		{
			std::string dir_name = "new_directory";
			while (exists(m_current_path / dir_name))
			{
				dir_name += "_1";
			}
			create_directory(m_current_path / "new_directory");
		}

		if (ImGui::Button("Scene", {50, 0} ))
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
		}

		ImGui::EndChild();
	}

	static fs::path selected;
	static bool renaming = false;
	static char renaming_buf[100];

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
					(renaming_buf, entry.path().filename().string().c_str());
					renaming = false;
				}
			}
			else
				ImGui::Text(entry.path().filename().string().c_str());
		}
		else
		{
			ImGui::PushID(i);
			if (ImGui::Selectable("F", selected == entry.path(), 0, {50, 50}))
			{
				selected = entry.path();
			}
			ImGui::PopID();
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

			//LOGDEBUGF("{}", (int)(ImGui::GetWindowSize().x / 50));

			//ImGui::SameLine();
			//if (i % (int)(ImGui::GetWindowSize().x / 50) != 0)
			//	ImGui::SetCursorPos({pos.x + 60, pos.y + 5});
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
