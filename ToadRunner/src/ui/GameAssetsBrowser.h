#pragma once

namespace project
{
	struct ProjectSettings;
}

namespace ui
{
	namespace fs = std::filesystem;

	class GameAssetsBrowser
	{
	public:
		GameAssetsBrowser(const std::filesystem::path& asset_path);
		~GameAssetsBrowser();

		static std::filesystem::path FindAssetPath(const project::ProjectSettings& project);

		void Show();
		void SetAssetPath(const std::filesystem::path& path);
		const fs::path& GetAssetPath() const;

		bool loaded_scene = false;
		bool refresh = false;

	private:
		fs::path m_assetsPath;
		fs::path m_currentPath;
		fs::path m_gameVsprojFile;
		fs::path m_gameScriptRegisterFile;

		std::vector<fs::path> m_current_path_contents = {};
		bool m_styleBlocks = true;

		// Create and add a toad script 
		bool CreateCPPScript(std::string_view script_name);

		// Add a script to the projects script registry (ScriptRegister.cpp)
		bool AddToScriptRegistry(const fs::path& script_path) const;

		// Remove a script from the projects script registry (ScriptRegister.cpp)
		bool RemoveFromScriptRegistry(const fs::path& script_path) const;

		// Include file to visual studio project
		bool IncludeToProjectFile(const fs::path& file_path_full);

		// Exclude file from visual studio project
		bool ExcludeToProjectFile(const fs::path& file_path_full);

		// Verify code project files and update paths
		bool VerifyPaths();
	};

}
