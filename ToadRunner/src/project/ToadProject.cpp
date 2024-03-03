#include "pch.h"
#include "ToadProject.h"

#include "engine/Engine.h"
#include "Misc.h"

#include "nlohmann/json.hpp"

#include <filesystem>
#include <fstream>

namespace project {

	namespace fs = std::filesystem;
	using json = nlohmann::json;

#ifdef _WIN32
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
#endif

	CREATE_PROJECT_RES_INFO Create(const ProjectSettings& settings)
	{
#ifdef _WIN32
        std::string premake5 = "premake5.exe";
#else
        std::string premake5 = "premake5";
#endif
		// verify settings paths 
		if (!fs::is_directory(settings.project_path))
		{
			return 
			{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Directory {} doesn't exist for project", settings.project_path)
			};
		}

#ifdef _WIN32
		if (!fs::is_empty(settings.project_path))
		{
			// change to warning, choose to add/overwrite directory contents
			return 
			{
				CREATE_PROJECT_RES::ERROR,
				Toad::format_str("Project directory isn't empty {}", settings.project_path)
			};
		}
#else
        if (!fs::is_empty(settings.project_path))
        {
            for (const auto& entry : fs::directory_iterator(settings.project_path))
            {
                if (entry.is_directory())
                {
                    // change to warning, choose to add/overwrite directory contents
                    return
                    {
                        CREATE_PROJECT_RES::ERROR,
                        Toad::format_str("Project directory isn't empty {}", settings.project_path)
                    };
                }
            }
        }
#endif

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

		// copy premake executable, only needed for windows
#ifdef _WIN32
		if (!fs::copy_file(engine_parent_path.string() + "/vendor/bin/" + premake5, settings.project_path + "/" + premake5))
		{
			return {
				CREATE_PROJECT_RES::ERROR,
				Toad::format_str("Failed to copy {} to {}", premake5, settings.project_path)
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
#else
        std::string project_path_backslash = settings.project_path;
#endif
        std::string proj_type_str = ProjectTypeAsStr(settings.project_gen_type);
		std::string command1 = Toad::format_str("cd {}", project_path_backslash);
		std::string command2 = Toad::format_str("{} --enginepath={} --projectname={} {}", premake5, settings.engine_path, settings.name, proj_type_str);
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

		// TODO: Test 
		for (const auto& entry : fs::recursive_directory_iterator(runner_src_path))
		{
			if (entry.path().filename() == "entry.cpp")
			{
				std::fstream f(entry.path().string());
				std::string s; 
				while (std::getline(f, s))
				{
					if (s.find("#include ") != std::string::npos)
					{
						if (auto pos = s.find("/Game/"); pos != std::string::npos)
						{
							s.replace(pos, std::string("/Game/").length(), '/' + settings.name + "_Game/");
							break;
						}
					}
				}
			}
		}

		// Vendor
		for (const auto& entry : fs::recursive_directory_iterator(fs::path(engine_parent_path / "vendor")))
		{
			// ignore /vendor/bin /examples /imgui/misc /docs /doc
			auto strpath = entry.path().string();
			if (strpath.find(fs::path("/vendor/bin").string()) != std::string::npos ||
				strpath.find(fs::path("/examples").string()) != std::string::npos ||
				strpath.find(fs::path("/imgui/misc").string()) != std::string::npos ||
				strpath.find(fs::path("/docs").string()) != std::string::npos ||
				strpath.find(fs::path("/doc").string()) != std::string::npos
				)
			{
				continue;
			}

			if (entry.is_directory())
			{
				auto pos = entry.path().string().find(fs::path("/vendor").string());
				if (pos != std::string::npos)
				{
					std::string relative = entry.path().string().substr(pos);
					fs::create_directories(settings.project_path + relative);
				}

			}
			else if (entry.is_regular_file())
			{
				auto pos = entry.path().string().find(fs::path("/vendor").string());
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

		json data = json::object();
		data["name"] = settings.name;

		std::ofstream engine_file(fs::path(settings.project_path) / (settings.name + FILE_EXT_TOADPROJECT));

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
			if (fs::path(path).extension() != FILE_EXT_TOADPROJECT)
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
                    settings.project_path = fs::path(path).parent_path().string();
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
				if (entry.path().extension() == FILE_EXT_TOADPROJECT)
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

			std::ifstream project_file(path_to_file);

			if (project_file.is_open())
			{
				try
				{
                    json data = json::parse(project_file);
					settings.name = data["name"];
                    settings.project_path = fs::path(path).parent_path().string();
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

	bool Update(const ProjectSettings& settings, const std::filesystem::path& path)
	{
		if (!fs::exists(path))
		{
			LOGERRORF("[Project] {} doesn't exist", path);
			return false;
		}

		fs::path project_file = path;

		if (fs::is_directory(path))
		{
			for (const auto& entry : fs::directory_iterator(path))
			{
				if (entry.path().extension() == FILE_EXT_TOADPROJECT)
				{
					project_file = entry.path();
					break;
				}
			}
		}

		if (fs::is_directory(project_file))
		{
			LOGERRORF("[Project] Can't find project file in {}", path);
			return false;
		}

		// parse project json for name 

		// setup parameters for premake 

		// find premake 

		// run premake with parameters --enginepath= --projectname=
		settings.engine_path;
		
	}

	std::string ProjectTypeAsStr(PROJECT_TYPE r)
    {
        switch(r){
            case PROJECT_TYPE::VS_2022:
                return "vs2022";
                break;
            case PROJECT_TYPE::VS_2019:
                return "vs2019";
                break;
            case PROJECT_TYPE::VS_2015:
                return "vs2015";
                break;
            case PROJECT_TYPE::Makefile:
                return "gmake2";
                break;
            case PROJECT_TYPE::Codelite:
                return "codelite";
                break;
            case PROJECT_TYPE::Xcode:
                return "xcode4";
                break;
            default:
                return "invalid";
                break;
        }
    }

}