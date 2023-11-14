#include "pch.h"
#include "ToadProject.h"

#include "engine/FormatStr.h"

#include "generate_game_project.lua.h"

#include <filesystem>
#include <fstream>

namespace project {

	namespace fs = std::filesystem;

	CREATE_PROJECT_RES_INFO Create(const ProjectSettings& settings)
	{
		// verify settings paths 
		if (!fs::is_directory(settings.project_path))
		{
			return {
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Directory {} doesn't exist for project", settings.project_path)
			};
		}

		if (!fs::is_empty(settings.project_path))
		{
			// change to warning, choose to add/overwrite directory contents
			return {
				CREATE_PROJECT_RES::ERROR,
				Toad::format_str("Project directory isn't empty {}", settings.project_path)
			};
		}

		if (!fs::is_directory(settings.engine_path))
		{
			return {
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Directory {} doesn't exist for engine path", settings.engine_path)
			};
		}

		fs::path engine_path_fs = settings.engine_path;

		// get engine parent folder and verify contents
		if (!engine_path_fs.has_parent_path())
		{
			return {
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have parent : {}", settings.engine_path)
			};
		}

		fs::path engine_parent_path = engine_path_fs.parent_path();

		// check for game, runner and vendor (and engine/src/pch.h?) 
		if (!fs::is_directory(engine_parent_path.string() + "Game"))
		{
			return {
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have Game/ folder in parent : {}", settings.engine_path)
			};
		}
		if (!fs::is_directory(engine_parent_path.string() + "ToadRunner"))
		{
			return {
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have ToadRunner/ folder in parent : {}", settings.engine_path)
			};
		}
		if (!fs::is_directory(engine_parent_path.string() + "Vendor"))
		{
			return {
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have Vendor/ folder in parent : {}", settings.engine_path)
			};
		}

		// create premake lua file 
		std::ofstream premake_lua_file(settings.project_path + "premake5.lua");

		if (!premake_lua_file)
		{
			return {
				CREATE_PROJECT_RES::ERROR,
				("Failed to create premake file")
			};
		}

		premake_lua_file << generate_game_project_lua;
		premake_lua_file.close();

		// copy premake executable
		if (!fs::copy_file("premake5.exe", settings.project_path))
		{
			return {
				CREATE_PROJECT_RES::ERROR,
				("Failed to copy premake5.exe to {}", settings.project_path)
			};
		}

		system(Toad::format_str("cd {}", settings.project_path).c_str());

		system(Toad::format_str("premake5.exe --enginepath={} --projectname={} vs2022", settings.engine_path, settings.name).c_str());

		// copy default game files 
		std::string runner_src_path = settings.project_path + "ToadRunner" + PATH_SEPARATOR + "src" + PATH_SEPARATOR;
		std::string engine_src_path = settings.project_path + "Engine" + PATH_SEPARATOR + "src" + PATH_SEPARATOR;
		std::string game_path = settings.project_path + "Game_" + settings.name + PATH_SEPARATOR;
		std::string vendor_path = settings.project_path + "vendor" + PATH_SEPARATOR;
		fs::create_directory(engine_src_path);
		fs::create_directory(runner_src_path);
		fs::create_directory(game_path);
		fs::create_directory(vendor_path);

		// Engine
		for (const auto& entry : fs::recursive_directory_iterator(engine_path_fs)) {
			if (entry.path().has_filename())
				if (entry.path().filename().string() == "pch.h")
				{
					/*engine_parent_path.string() + PATH_SEPARATOR + "src" + PATH_SEPARATOR + "pch.h"*/
					fs::copy_file(entry.path().string(), engine_src_path);
					break;
				}
		}
		
		// Game
		fs::copy(engine_parent_path.string() + "Game" + PATH_SEPARATOR + "src", game_path);

		// ToadRunner
		fs::copy(engine_parent_path.string() + "ToadRunner" + PATH_SEPARATOR + "src", runner_src_path);

		// Vendor
		fs::copy(engine_parent_path.string() + "vendor", settings.project_path);

		// open in visual studio
		system(Toad::format_str("{}.sln", settings.name).c_str());
	}

}