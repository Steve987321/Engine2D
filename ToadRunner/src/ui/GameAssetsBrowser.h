#pragma once

#include <filesystem>
namespace Toad
{
	namespace fs = std::filesystem;

	class GameAssetsBrowser
	{
	public:
		GameAssetsBrowser(std::string_view asset_path);
		~GameAssetsBrowser();

		void Show();
		void SetAssetPath(std::string_view path);
		const fs::path& GetAssetPath();

	private:
		fs::path m_assets_path;
		fs::path m_current_path;
	};

}
