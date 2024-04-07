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

		bool loaded_scene = false;
		bool refresh = false;

	private:
		fs::path m_assetsPath;
		fs::path m_currentPath;
		fs::path m_gameVsprojFile;
		fs::path m_gameScriptRegisterFile;

		std::vector<fs::path> m_current_path_contents = {};
		bool m_styleBlocks = true;

		bool CreateCPPScript(std::string_view script_name);
		bool AddToScriptRegistry(const fs::path& script_path) const;
		bool RemoveFromScriptRegistry(const fs::path& script_path) const;
		bool IncludeToProjectFile(const fs::path& file_path_full);
		bool ExcludeToProjectFile(const fs::path& file_path_full);
		bool VerifyPaths();
	};

}
