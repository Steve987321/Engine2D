#include "pch.h"

#include "Engine/Engine.h"

#include "GameAssetsBrowser.h"

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
}

void GameAssetsBrowser::SetAssetPath(std::string_view path)
{
	m_assets_path = path;
}

}
