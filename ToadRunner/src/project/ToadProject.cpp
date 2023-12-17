#include "pch.h"
#include "ToadProject.h"

#include "engine/FormatStr.h"
#include "Misc.h"

#include "nlohmann/json.hpp"

#include <filesystem>
#include <fstream>

namespace project {

	namespace fs = std::filesystem;
	using json = nlohmann::json;

	bool OpenSln(const fs::path& path, const misc::Editor& editor)
	{
		if (!path.has_extension() || path.extension() != ".sln")
			return false;

		STARTUPINFOA si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		if (!CreateProcessA(NULL,
			const_cast<LPSTR>((editor.path.string() + ' ' + path.string()).c_str()),
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			NULL,
			&si,
			&pi))
		{
			return false;
		}

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return true;
	}

	CREATE_PROJECT_RES_INFO Create(const ProjectSettings& settings)
	{
		// verify settings paths 
		if (!fs::is_directory(settings.project_path))
		{
			return 
			{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Directory {} doesn't exist for project", settings.project_path)
			};
		}

		if (!fs::is_empty(settings.project_path))
		{
			// change to warning, choose to add/overwrite directory contents
			return 
			{
				CREATE_PROJECT_RES::ERROR,
				Toad::format_str("Project directory isn't empty {}", settings.project_path)
			};
		}

		if (!fs::is_directory(settings.engine_path))
		{
			return 
			{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Directory {} doesn't exist for engine path", settings.engine_path)
			};
		}

		fs::path engine_path_fs = settings.engine_path;

		// get engine parent folder and verify contents
		if (!engine_path_fs.has_parent_path())
		{
			return 
			{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have parent : {}", settings.engine_path)
			};
		}

		fs::path engine_parent_path = engine_path_fs.parent_path();

		// check for game, runner and vendor (and engine/src/pch.h?) 
 		if (!fs::is_directory(engine_parent_path.string() + "/Game"))
		{
			return
			{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have Game/ folder in parent : {}", settings.engine_path)
			};
		}
		if (!fs::is_directory(engine_parent_path.string() + "/ToadRunner"))
		{
			return 
			{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have ToadRunner/ folder in parent : {}", settings.engine_path)
			};
		}
		if (!fs::is_directory(engine_parent_path.string() + "/Vendor"))
		{
			return
			{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have Vendor/ folder in parent : {}", settings.engine_path)
			};
		}

		std::ifstream generate_game_project_lua(engine_parent_path / "ToadRunner" / "src" / "project" / "generate_game_project.lua");

		if (!generate_game_project_lua.is_open())
		{
			return 
			{
				   CREATE_PROJECT_RES::ERROR,
				   ("Failed to open premake file")
			};
		}

		std::stringstream generate_game_project_str;
		generate_game_project_str << generate_game_project_lua.rdbuf();

		generate_game_project_lua.close();

		// create premake lua file 
		std::ofstream premake_lua_file(settings.project_path + "/premake5.lua");

		if (!premake_lua_file)
		{
			return 
			{
				CREATE_PROJECT_RES::ERROR,
				("Failed to create premake file")
			};
		}

		premake_lua_file << generate_game_project_str.str();
		premake_lua_file.close();

		// copy premake executable
		if (!fs::copy_file(engine_parent_path.string() + "/vendor/bin/premake5.exe", settings.project_path + "/premake5.exe"))
		{
			return {
				CREATE_PROJECT_RES::ERROR,
				("Failed to copy premake5.exe to {}", settings.project_path)
			};
		}
		std::string project_path_backslash;
		for (char c : settings.project_path)
		{
			if (c == '/')
			{
				project_path_backslash += '\\';
				continue;
			}
			project_path_backslash += c;
		}

		std::string command1 = Toad::format_str("cd {}", project_path_backslash);
		std::string command2 = Toad::format_str("premake5.exe --enginepath={} --projectname={} vs2022", settings.engine_path, settings.name);
		int res = system(Toad::format_str("{} && {}", command1, command2).c_str());
		if (res == -1)
		{
			return 
			{
				CREATE_PROJECT_RES::ERROR,
				("Failed to execute command {}", Toad::format_str("{} && {}", command1, command2))
			};
		}

		// copy default game files 
		std::string runner_src_path = settings.project_path + "/ToadRunner/src";
		std::string engine_src_path = settings.project_path + "/Engine/src";
		std::string game_path = settings.project_path + "/" + settings.name + "_Game/src";
		std::string vendor_path = settings.project_path + "/vendor";
		try
		{
			fs::create_directory(engine_src_path);
			fs::create_directory(runner_src_path);
			fs::create_directory(game_path);
			fs::create_directory(vendor_path);
		}
		catch(fs::filesystem_error e)
		{
			std::cout << e.what() << std::endl;
		}

		// Engine
		for (const auto& entry : fs::recursive_directory_iterator(engine_path_fs)) {
			if (entry.path().has_filename())
				if (entry.path().filename().string() == "pch.h")
				{
					fs::copy_file(entry.path().string(), engine_src_path + "/pch.h");
					break;
				}
		}
		
		// Game
		fs::copy(engine_parent_path.string() + "/Game/src", game_path, fs::copy_options::overwrite_existing | fs::copy_options::recursive);

		// ToadRunner
		fs::copy(engine_parent_path.string() + "/ToadRunner/src", runner_src_path, fs::copy_options::overwrite_existing | fs::copy_options::recursive);

		// Vendor
		for (const auto& entry : fs::recursive_directory_iterator(fs::path(engine_parent_path / "vendor")))
		{
			// ignore /vendor/bin /examples /imgui/misc /docs /doc
			auto strpath = entry.path().string();
			if (strpath.find("\\vendor\\bin") != std::string::npos ||
				strpath.find("\\examples") != std::string::npos ||
				strpath.find("\\imgui\\misc") != std::string::npos ||
				strpath.find("\\docs") != std::string::npos ||
				strpath.find("\\doc") != std::string::npos
				)
			{
				continue;
			}

			if (entry.is_directory())
			{
				auto pos = entry.path().string().find("\\vendor");
				if (pos != std::string::npos)
				{
					std::string relative = entry.path().string().substr(pos);
					fs::create_directories(settings.project_path + relative);
				}

			}
			else if (entry.is_regular_file())
			{
				auto pos = entry.path().string().find("\\vendor");
				if (pos != std::string::npos)
				{
					std::string relative = entry.path().string().substr(pos);
					fs::copy_file(entry.path(), settings.project_path + relative, fs::copy_options::overwrite_existing);
				}
			}
		}

		// run premake again for new files 
		res = system(Toad::format_str("{} && {}", command1, command2).c_str());
		if (res == -1)
		{
			return 
			{
				CREATE_PROJECT_RES::ERROR,
				Toad::format_str("Failed to execute command {}", Toad::format_str("{} && {}", command1, command2))
			};
		}

		// project file 

		json data;
		data["name"] = settings.name;
		data["engine_path"] = settings.engine_path;
		data["project_path"] = settings.project_path;

		std::ofstream engine_file(fs::path(settings.project_path) / (settings.name + ".TOADPROJECT"));

		if (engine_file.is_open())
		{
			engine_file << std::setw(4) << data;
			engine_file.close();
		}

		auto load_proj_res = Load(settings.project_path);
		if (load_proj_res.res != LOAD_PROJECT_RES::OK)
		{
			return
			{
				CREATE_PROJECT_RES::ERROR,
				Toad::format_str("Failed to load project, {}", load_proj_res.description)
			};
		}

		// open in visual studio

		if (!misc::current_editor.name.empty())
		{
#if 0
			if (!OpenSln(settings))
			{
				return
				{
					CREATE_PROJECT_RES::ERROR,
					(Toad::format_str("Failed to create {} instance", misc::current_editor.name))
				};
			}
#endif
			return
			{
				CREATE_PROJECT_RES::OK,
				Toad::format_str("Created project {}", settings.name)
			};
		}
		else
		{
			return 
			{
				CREATE_PROJECT_RES::OK,
				Toad::format_str("Created project {}", settings.name)
			};
		}
	}

	// sets current_project 
	LOAD_PROJECT_RES_INFO Load(const std::string_view path)
	{
		ProjectSettings settings;

		if (!fs::exists(path))
		{
			return
			{
				 LOAD_PROJECT_RES::DOESNT_EXIST,
				 Toad::format_str("{} doesn't exist", path)
			};
		}

		if (fs::is_regular_file(path))
		{
			if (fs::path(path).extension() != ".TOADPROJECT")
			{
				return
				{
					LOAD_PROJECT_RES::INVALID_PROJECT_FILE,
					Toad::format_str("{} isn't a valid project file extension", path)
				};
			}

			std::ifstream project_file(path.data());

			if (project_file.is_open())
			{
				try
				{
					json data = json::parse(project_file);

					settings.name = data["name"];
					settings.engine_path = data["engine_path"];
					settings.project_path = data["project_path"];
				}
				catch(json::parse_error& e)
				{
					project_file.close();

					return
					{
						LOAD_PROJECT_RES::INVALID_PROJECT_FILE,
						Toad::format_str("Parse error at {}, {}. While parsing {}", e.byte, e.what(), path)
					};
				}

				project_file.close();
			}
		}

		if (fs::is_directory(path))
		{
			fs::path path_to_file;

			for (const auto& entry : fs::directory_iterator(path))
			{
				if (entry.path().extension() == ".TOADPROJECT")
				{
					path_to_file = entry.path();
					break;
				}
			}

			if (path_to_file.empty())
			{
				return
				{
					LOAD_PROJECT_RES::DOESNT_EXIST,
					Toad::format_str("Failed to find project file in {}", path)
				};
			}

			std::ifstream project_file(path.data());

			if (project_file.is_open())
			{
				json data = json::parse(project_file);

				try
				{
					settings.name = data["name"];
					settings.engine_path = data["engine_path"];
					settings.project_path = data["project_path"];
				}
				catch (json::parse_error& e)
				{
					project_file.close();

					return
					{
						LOAD_PROJECT_RES::INVALID_PROJECT_FILE,
						Toad::format_str("Parse error at {}, {}. While parsing {}", e.byte, e.what(), path)
					};
				}

				project_file.close();
			}

		}	

		current_project = settings;

		return
		{
			LOAD_PROJECT_RES::OK,
			Toad::format_str("Loaded project {} successfully", settings.name)
		};
	}

}