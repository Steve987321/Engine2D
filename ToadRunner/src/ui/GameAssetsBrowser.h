#pragma once

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
		fs::path m_game_vsproj_file;
		fs::path m_game_script_register_file;

		bool CreateCPPScript(std::string_view script_name);
		bool AddToScriptRegistry(const fs::path& script_path) const;
		bool RemoveFromScriptRegistry(const fs::path& script_path) const;
		bool IncludeToProjectFile(const fs::path& file_path_full);
		bool ExcludeToProjectFile(const fs::path& file_path_full);
		bool VerifyPaths();
	};

}
