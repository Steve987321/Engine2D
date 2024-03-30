#include "pch.h"
#include "ToadProject.h"

#include "engine/Engine.h"
#include "Misc.h"

#include "nlohmann/json.hpp"

#include "engine/Helpers.h"

#include <filesystem>
#include <fstream>

namespace project {

	namespace fs = std::filesystem;
	using json = nlohmann::json;

	// '/' to '\'
	void path_as_backslash(std::string& s)
	{
		for (char& c : s)
		{
			if (c == '/')
				c = '\\';
		}
	}

	// '\' to '/'
	void path_as_forward_slash(std::string& s)
	{
		for (char& c : s)
		{
			if (c == '\\')
				c = '/';
		}
	}

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

	// very minimal checking 
	extern bool VerifyEnginePathContents(const fs::path& path, CREATE_PROJECT_RES_INFO& ri);

	CREATE_PROJECT_RES_INFO Create(const ProjectSettings& settings)
	{
		std::string project_path_forwardslash = settings.project_path;
		std::string engine_path_forwardslash = settings.engine_path;
		std::string proj_type_str = ProjectTypeAsStr(settings.project_gen_type);

		path_as_forward_slash(engine_path_forwardslash);
		path_as_forward_slash(project_path_forwardslash);

#ifdef _WIN32
		const std::string premake5 = "premake5.exe";
#else
		const std::string premake5 = "premake5";
#endif
		const fs::path engine_path_fs = settings.engine_path;

#ifdef TOAD_DISTRO
		const fs::path generate_game_lua = engine_path_fs / "scripts" / "generate_game_project.lua";
		const fs::path bin_path = engine_path_fs.string() + "/bin/" + premake5;
#else
		const fs::path generate_game_lua = engine_path_fs.parent_path() / "ToadRunner" / "src" / "project" / "generate_game_project.lua";
		const fs::path bin_path = engine_path_fs.parent_path().string() + "/vendor/bin/" + premake5;
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

		CREATE_PROJECT_RES_INFO ri;
		if (!VerifyEnginePathContents(settings.engine_path, ri))
		{
			return ri;
		}

		fs::copy_file(generate_game_lua, settings.project_path + "/premake5.lua");
		
		// copy premake executable, only needed for windows
#ifdef _WIN32
		if (!fs::copy_file(bin_path, settings.project_path + '/' + premake5))
		{
			return {
				CREATE_PROJECT_RES::ERROR,
				Toad::format_str("Failed to copy {} to {}", premake5, settings.project_path)
			};
		}
#endif // _WIN32

#ifdef TOAD_DISTRO
		std::string command = Toad::format_str("{} {} --file={} --enginepath={} --projectname={}", 
			project_path_forwardslash + '/' + premake5,
			proj_type_str, 
			project_path_forwardslash + "/premake5.lua",
			engine_path_forwardslash,
			settings.name);
#else
		std::string command = Toad::format_str("{} {} --file={} --enginepath={} --projectname={} --usesrc",
			project_path_forwardslash + '/' + premake5,
			proj_type_str, 
			project_path_forwardslash + "/premake5.lua",
			engine_path_forwardslash,
			settings.name);
#endif
		LOGDEBUG(command.c_str());
		int res = system(command.c_str());
		if (res == -1)
		{
			return 
			{
				CREATE_PROJECT_RES::ERROR,
				("Failed to execute command {}", command)
			};
		}

		// create/copy default game/engine files 

		std::string game_path = project_path_forwardslash + "/" + settings.name + "_Game/src";
#ifdef TOAD_DISTRO
		try
		{
			fs::create_directory(game_path);
		}
		catch (fs::filesystem_error e)
		{
			std::cout << e.what() << std::endl;
		}

		fs::copy(engine_path_fs / "game_template" / "src", game_path, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
		fs::copy(engine_path_fs / "game_template" / "vendor", project_path_forwardslash + "/vendor", fs::copy_options::overwrite_existing | fs::copy_options::recursive);
#else
		std::string runner_src_path = project_path_forwardslash + "/ToadRunner/src";
		std::string engine_src_path = project_path_forwardslash + "/Engine/src";
		std::string vendor_path = project_path_forwardslash + "/vendor";
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
					fs::copy_file(entry.path().string(), fs::path(engine_src_path + "/pch.h"));
					break;
				}
		}
		
		// Game
		fs::copy(engine_path_fs.parent_path().string() + "/Game/src", game_path, fs::copy_options::overwrite_existing | fs::copy_options::recursive);

		// ToadRunner
		fs::copy(engine_path_fs.parent_path().string() + "/ToadRunner/src", runner_src_path, fs::copy_options::overwrite_existing | fs::copy_options::recursive);

		// Vendor
		for (const auto& entry : fs::recursive_directory_iterator(fs::path(engine_path_fs.parent_path() / "vendor")))
		{
			// ignore /vendor/bin /examples /imgui/misc /docs /doc
			auto strpath = entry.path().string();
			if (strpath.find(Toad::format_str("vendor{}bin", PATH_SEPARATOR)) != std::string::npos ||
				strpath.find("examples") != std::string::npos ||
				strpath.find(Toad::format_str("imgui{}misc", PATH_SEPARATOR)) != std::string::npos ||
				strpath.find("docs") != std::string::npos ||
				strpath.find("doc") != std::string::npos
				)
			{
				continue;
			}

			if (entry.is_directory())
			{
				// very weird?
				auto pos = entry.path().string().find("vendor");
				if (pos != std::string::npos)
				{
					std::string relative = entry.path().string().substr(pos);
					fs::create_directories(settings.project_path + PATH_SEPARATOR + relative);
				}

			}
			else if (entry.is_regular_file())
			{
				auto pos = entry.path().string().find("vendor");
				if (pos != std::string::npos)
				{
					std::string relative = entry.path().string().substr(pos);
					fs::copy_file(entry.path(), settings.project_path + PATH_SEPARATOR + relative, fs::copy_options::overwrite_existing);
				}
			}
		}
#endif
		// run premake again for new files 
		res = system(command.c_str());
		if (res == -1)
		{
			return 
			{
				CREATE_PROJECT_RES::ERROR,
				Toad::format_str("Failed to execute command {}", command)
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
				 Toad::format_str("{} doesn't exist", path),
				 settings
			};
		}

		if (fs::is_regular_file(path))
		{
			if (fs::path(path).extension() != FILE_EXT_TOADPROJECT)
			{
				return
				{
					LOAD_PROJECT_RES::INVALID_PROJECT_FILE,
					Toad::format_str("{} isn't a valid project file extension", path),
					settings
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
					
					GET_JSON_ELEMENT(settings.editor_cam_pos.x, data, "editor_cam_posx");
					GET_JSON_ELEMENT(settings.editor_cam_pos.y, data, "editor_cam_posy");
					GET_JSON_ELEMENT(settings.editor_cam_size.x, data, "editor_cam_sizex");
					GET_JSON_ELEMENT(settings.editor_cam_size.y, data, "editor_cam_sizex");
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

					GET_JSON_ELEMENT(settings.editor_cam_pos.x, data, "editor_cam_posx");
					GET_JSON_ELEMENT(settings.editor_cam_pos.y, data, "editor_cam_posy");
					GET_JSON_ELEMENT(settings.editor_cam_size.x, data, "editor_cam_sizex");
					GET_JSON_ELEMENT(settings.editor_cam_size.y, data, "editor_cam_sizex");
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

	project::PROJECT_TYPE DetectProjectType(const std::filesystem::path& proj_dir)
	{
		for (const auto& entry : fs::directory_iterator(proj_dir))
		{
			if (fs::is_directory(entry.path()))
				continue;

			if (entry.path().extension() == ".sln")
				return project::PROJECT_TYPE::VS_2022;
			if (entry.path().filename() == "Makefile")
				return project::PROJECT_TYPE::Makefile;
		}

#ifdef _WIN32
		return project::PROJECT_TYPE::VS_2022;
#elif __APPLE__
		return project::PROJECT_TYPE::Makefile;
#endif
	}

	bool Update(const ProjectSettings& settings, const std::filesystem::path& path)
	{
		if (!fs::exists(path))
		{
			LOGERRORF("[Project] {} doesn't exist", path);
			return false;
		}

		if (!fs::exists(settings.engine_path))
		{
			LOGERRORF("[Project] {} doesn't exist", settings.engine_path);
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

		std::ifstream project_file_contents(project_file);

		if (!project_file_contents)
		{
			LOGERRORF("[Project] Can't read {}", path);
			return false;
		}

		json data;
		try {
			data = json::parse(project_file_contents);
		}
		catch (json::parse_error& e) {
			LOGERRORF("[Project] Failed to parse {}, {}", path, e.what());
			return false;
		}
		
		std::string project_name;
		GET_JSON_ELEMENT(project_name, data, "name");

		if (project_name.empty())
		{
			LOGERRORF("[Project] Invalid project name in {}", project_file);
			return false;
		}

		std::string premake5;
#ifdef _WIN32
		// in project 
		premake5 = (project_file.parent_path() / "premake5.exe").string();
		if (!fs::exists(premake5))
		{
			LOGERRORF("[Project] Can't find premake5.exe in {}", project_file.parent_path());
			return false;
		}
#else
		premake5 = "premake5";
#endif

		PROJECT_TYPE proj_type = DetectProjectType(project_file.parent_path());

		// #TODO should it use the (updated) one in the engine directory or the one in the game directory which may be outdated ?
#ifdef TOAD_DISTRO
		std::string full_command = Toad::format_str(
			"{} {} --file={} --enginepath={} --projectname={}",
			premake5,
			ProjectTypeAsStr(proj_type),
			(project_file.parent_path() / "premake5.lua").string(),
			settings.engine_path,
			project_name);
#else
		std::string full_command = Toad::format_str(
			"{} {} --file={} --enginepath={} --projectname={} --usesrc",
			premake5,
			ProjectTypeAsStr(proj_type),
			(project_file.parent_path() / "premake5.lua").string(),
			settings.engine_path,
			project_name);
#endif
		LOGDEBUGF("Running command: {}", full_command);
		system(full_command.c_str());
		return true;
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

	bool VerifyEnginePathContents(const fs::path& path, CREATE_PROJECT_RES_INFO& ri)
	{
#ifdef TOAD_DISTRO
		// check for libs, script_api, vendor  
		if (!fs::is_directory(path.string() + "/bin"))
		{
			ri = CREATE_PROJECT_RES_INFO{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have libs/ folder in parent : {}", path)
			};

			return false;
		}
		if (!fs::is_directory(path.string() + "/libs"))
		{
			ri = CREATE_PROJECT_RES_INFO{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have libs/ folder in parent : {}", path)
			};

			return false;
		}
		if (!fs::is_directory(path.string() + "/script_api"))
		{
			ri = CREATE_PROJECT_RES_INFO{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have script_api/ folder in parent : {}", path)
			};

			return false;
		}
		if (!fs::is_directory(path.string() + "/game_template"))
		{
			ri = CREATE_PROJECT_RES_INFO{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have game_template/ folder in parent : {}", path)
			};

			return false;
		}
		if (!fs::is_directory(path.string() + "/scripts"))
		{
			ri = CREATE_PROJECT_RES_INFO{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have scripts/ folder in parent : {}", path)
			};

		}
#else
		// get engine parent folder and verify contents
		if (!path.has_parent_path())
		{
			ri = CREATE_PROJECT_RES_INFO{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have parent : {}", path)
			};

			return false;
		}

		fs::path engine_parent_path = path.parent_path();

		// check for game, runner and vendor (and engine/src/pch.h?) 
		if (!fs::is_directory(engine_parent_path.string() + "/Game"))
		{
			ri = CREATE_PROJECT_RES_INFO{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have Game/ folder in parent : {}", engine_parent_path)
			};

			return false;
		}
		if (!fs::is_directory(engine_parent_path.string() + "/ToadRunner"))
		{
			ri = CREATE_PROJECT_RES_INFO{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have ToadRunner/ folder in parent : {}", engine_parent_path)
			};

			return false;
		}
		if (!fs::is_directory(engine_parent_path.string() + "/Vendor"))
		{
			ri = CREATE_PROJECT_RES_INFO{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have Vendor/ folder in parent : {}", engine_parent_path)
			};

			return false;
		}
#endif
		return true;
	}

}