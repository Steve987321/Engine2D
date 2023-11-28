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
	if (m_assets_path.empty())
		return;

	list_dir_contents(m_assets_path);

	ImGui::Begin("second");

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
			for (const auto& e : editors)
			{
				if (e.name.find("Visual Studio") != std::string::npos)
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

					if (!project::OpenSln(slnpath, e))
					{
						LOGERRORF("Failed to open {} with {}", slnpath, e.name);
					}
					
					break;
				}
			}
		}
	}
	for (const auto& entry: fs::directory_iterator(m_current_path))
	{
		if (entry.is_directory())
		{
			if (ImGui::Button("D", { 50,50 }))
			{
				m_current_path = entry.path();
			}
			ImGui::Text(entry.path().filename().string().c_str());
		}
		else
		{
			if (ImGui::Button("F", {50, 50}))
			{
				
			}
			ImGui::Text(entry.path().filename().string().c_str());
		}
	}

	ImGui::End();
}

void GameAssetsBrowser::SetAssetPath(std::string_view path)
{
	m_assets_path = path;
	m_current_path = path;
}

}
