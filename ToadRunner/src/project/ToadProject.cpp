#include "pch.h"
#include "ToadProject.h"

#include "engine/Engine.h"
#include "Misc.h"

#include "ui/GameAssetsBrowser.h"

#include "nlohmann/json.hpp"

#include <filesystem>
#include <fstream>

namespace project {

	namespace fs = std::filesystem;
	using json = nlohmann::ordered_json;

	// '/' to '\'
	static void path_as_backslash(std::string& s)
	{
		for (char& c : s)
		{
			if (c == '/')
				c = '\\';
		}
	}

	// '\' to '/'
	static void path_as_forward_slash(std::string& s)
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

	CREATE_PROJECT_RES_INFO Create(const ProjectSettings& settings, const std::string& selected_template)
	{
		std::string project_path_forwardslash = settings.project_path.string();
		std::string engine_path_forwardslash = settings.engine_path.string();
		std::string proj_type_str = ProjectTypeAsStr(settings.project_gen_type);

		path_as_forward_slash(engine_path_forwardslash);
		path_as_forward_slash(project_path_forwardslash);

#ifdef _WIN32
		const std::string premake5 = "premake5.exe";
		const std::string full_path_to_premake5 = project_path_forwardslash + '/' + premake5;
#else
		std::string premake5 = "premake5";
		const std::string& full_path_to_premake5 = premake5;
#endif
		const fs::path engine_path_fs = settings.engine_path;

#ifdef TOAD_DISTRO
		const fs::path generate_game_lua = engine_path_fs / "scripts" / "generate_game_project.lua";
		const fs::path bin_path = engine_path_fs.string() + "/bin/" + premake5;
#else
		const fs::path generate_game_lua = engine_path_fs / "ToadRunner" / "src" / "project" / "generate_game_project.lua";
		const fs::path bin_path = engine_path_fs.string() + "/vendor/bin/" + premake5;
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

		fs::copy_file(generate_game_lua, settings.project_path / "premake5.lua", fs::copy_options::overwrite_existing);
		
		// copy premake executable, only needed for windows
#ifdef _WIN32
		if (!fs::copy_file(bin_path, settings.project_path / premake5))
		{
			return {
				CREATE_PROJECT_RES::ERROR,
				Toad::format_str("Failed to copy {} to {}", premake5, settings.project_path)
			};
		}
#endif // _WIN32

//#ifdef TOAD_DISTRO
//		std::string command = Toad::format_str("{} {} --file={} --enginepath={} --projectname={}", 
//			project_path_forwardslash + '/' + premake5,
//			proj_type_str, 
//			project_path_forwardslash + "/premake5.lua",
//			engine_path_forwardslash,
//			settings.name);
//#else
//		std::string command = Toad::format_str("{} {} --file={} --enginepath={} --projectname={} --usesrc",
//			project_path_forwardslash + '/' + premake5,
//			proj_type_str,
//			project_path_forwardslash + "/premake5.lua",
//			engine_path_forwardslash,
//			settings.name);
//#endif

#ifdef TOAD_DISTRO
		std::string premake_use_src_arg = "";
#else
		std::string premake_use_src_arg = "--usesrc";
#endif
		std::string command = Toad::format_str("{} {} --file={} --enginepath={} --projectname={} {}",
			full_path_to_premake5,
			proj_type_str,
			project_path_forwardslash + "/premake5.lua",
			engine_path_forwardslash,
			settings.name,
			premake_use_src_arg);

		LOGDEBUGF("{}", command.c_str());
		int res = system(command.c_str());
		LOGDEBUGF("result: {}", res);
		if (res != 0)
		{
#ifdef __APPLE__
            // try using the full path
            if (res == 32512)
            {
                // for now its just this
                premake5 = "/opt/homebrew/bin/premake5";
                
                // fix the command
                command = Toad::format_str("{} {} --file={} --enginepath={} --projectname={} {}",
                    full_path_to_premake5,
                    proj_type_str,
                    project_path_forwardslash + "/premake5.lua",
                    engine_path_forwardslash,
                    settings.name,
                    premake_use_src_arg);
                
                int res = system(command.c_str());
                LOGDEBUGF("result: {}", res);
                if (res != 0)
                {
                    return
                    {
                        CREATE_PROJECT_RES::ERROR,
                        Toad::format_str("Failed to execute command {}", command)
                    };
                }
            }
#else
			return
			{
				CREATE_PROJECT_RES::ERROR,
				Toad::format_str("Failed to execute command {}", command)
			};
#endif
		}

		// create/copy default game/engine files 

		std::string game_path = project_path_forwardslash + "/" + settings.name + "_Game/src";
//#ifdef TOAD_DISTRO
		try
		{
			fs::create_directories(game_path);
		}
		catch (fs::filesystem_error e)
		{
			std::cout << e.what() << std::endl;
		}

		// vendor is global for now 
#ifdef TOAD_DISTRO
		fs::copy(engine_path_fs / "game_templates" / selected_template / "src", game_path, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
		fs::copy(engine_path_fs / "game_templates" / "vendor", project_path_forwardslash + "/vendor", fs::copy_options::overwrite_existing | fs::copy_options::recursive);
#else
		fs::copy(engine_path_fs / "GameTemplates" / selected_template / "src", game_path, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
		for (const auto& e : fs::directory_iterator(engine_path_fs / "vendor"))
			if (e.path().filename().string().find("imgui") != std::string::npos ||
				e.path().filename().string().find("json") != std::string::npos ||
				e.path().filename().string().find("SFML-2.6") != std::string::npos ||
				e.path().filename().string().find("sfml-imgui") != std::string::npos || 
				e.path().filename().string().find("filewatch") != std::string::npos)
			{
				for (const auto& e2 : fs::recursive_directory_iterator(e.path()))
				{
					if (!e2.is_regular_file())
						continue; 

					std::string ext = e2.path().filename().extension().string();

					if (ext == ".h" || ext == ".cpp" || ext == ".hpp" || ext == ".inl" || ext == ".lib" || ext == ".dylib")
					{
						fs::path destination = fs::path(settings.project_path) / "vendor" / fs::relative(e2.path(), engine_path_fs / "Vendor").string();
						if (!fs::exists(destination.parent_path()))
							fs::create_directories(destination.parent_path());

						fs::copy_file(e2.path(), destination, fs::copy_options::overwrite_existing);
					}
				}
			}
		//fs::copy(engine_path_fs / "Game" / "vendor", project_path_forwardslash + "/vendor", fs::copy_options::overwrite_existing | fs::copy_options::recursive);
#endif 
//#else
		//std::string runner_src_path = project_path_forwardslash + "/ToadRunner/src";
		//std::string engine_src_path = project_path_forwardslash + "/Engine/src";
		//std::string vendor_path = project_path_forwardslash + "/vendor";
		//try
		//{
		//	fs::create_directory(engine_src_path);
		//	fs::create_directory(runner_src_path);
		//	fs::create_directory(game_path);
		//	fs::create_directory(vendor_path);
		//}
		//catch(fs::filesystem_error e)
		//{
		//	std::cout << e.what() << std::endl;
		//}

		//// Engine
		//for (const auto& entry : fs::recursive_directory_iterator(engine_path_fs)) {
		//	if (entry.path().has_filename())
		//		if (entry.path().filename().string() == "pch.h")
		//		{
		//			fs::copy_file(entry.path().string(), fs::path(engine_src_path + "/pch.h"));
		//			break;
		//		}
		//}
		//
		//// Game
		//fs::copy(engine_path_fs.string() + "/Game/src", game_path, fs::copy_options::overwrite_existing | fs::copy_options::recursive);

		//// ToadRunner
		//fs::copy(engine_path_fs.string() + "/ToadRunner/src", runner_src_path, fs::copy_options::overwrite_existing | fs::copy_options::recursive);

		//// Vendor
		//for (const auto& entry : fs::recursive_directory_iterator(fs::path(engine_path_fs / "vendor")))
		//{
		//	// ignore /vendor/bin /examples /imgui/misc /docs /doc
		//	auto strpath = entry.path().string();
		//	if (strpath.find(Toad::format_str("vendor{}bin", PATH_SEPARATOR)) != std::string::npos ||
		//		strpath.find("examples") != std::string::npos ||
		//		strpath.find(Toad::format_str("imgui{}misc", PATH_SEPARATOR)) != std::string::npos ||
		//		strpath.find("docs") != std::string::npos ||
		//		strpath.find("doc") != std::string::npos
		//		)
		//	{
		//		continue;
		//	}

		//	if (entry.is_directory())
		//	{
		//		// very weird?
		//		auto pos = entry.path().string().find("vendor");
		//		if (pos != std::string::npos)
		//		{
		//			std::string relative = entry.path().string().substr(pos);
		//			fs::create_directories(settings.project_path + PATH_SEPARATOR + relative);
		//		}

		//	}
		//	else if (entry.is_regular_file())
		//	{
		//		auto pos = entry.path().string().find("vendor");
		//		if (pos != std::string::npos)
		//		{
		//			std::string relative = entry.path().string().substr(pos);
		//			fs::copy_file(entry.path(), settings.project_path + PATH_SEPARATOR + relative, fs::copy_options::overwrite_existing);
		//		}
		//	}
		//}
//#endif
		// run premake again for new files 

		res = system(command.c_str());
		if (res != 0)
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

		auto load_proj_res = Load(settings.project_path.string());
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
	LOAD_PROJECT_RES_INFO Load(const fs::path& path)
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

			std::ifstream project_file(path);

			if (project_file.is_open())
			{
				try
				{
					json data = json::parse(project_file);

					settings.name = data["name"];
                    settings.project_path = fs::path(path).string();
					
					GET_JSON_ELEMENT(settings.editor_cam_pos.x, data, "editor_cam_posx");
					GET_JSON_ELEMENT(settings.editor_cam_pos.y, data, "editor_cam_posy");
					GET_JSON_ELEMENT(settings.editor_cam_size.x, data, "editor_cam_sizex");
					GET_JSON_ELEMENT(settings.editor_cam_size.y, data, "editor_cam_sizex");					
				}
				catch(json::parse_error& e)
				{
					project_file.close();

					current_project = settings;

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
                    settings.project_path = fs::path(path).string();

					GET_JSON_ELEMENT(settings.editor_cam_pos.x, data, "editor_cam_posx");
					GET_JSON_ELEMENT(settings.editor_cam_pos.y, data, "editor_cam_posy");
					GET_JSON_ELEMENT(settings.editor_cam_size.x, data, "editor_cam_sizex");
					GET_JSON_ELEMENT(settings.editor_cam_size.y, data, "editor_cam_sizex");
				}
				catch (json::parse_error& e)
				{
					project_file.close();

					current_project = settings;

					return
					{
						LOAD_PROJECT_RES::INVALID_PROJECT_FILE,
						Toad::format_str("Parse error at {}, {}. While parsing {}", e.byte, e.what(), path)
					};
				}

				project_file.close();
			}

		}	

		// clear possible previous resources
		Toad::ResourceManager::Clear();

		current_project = settings;
		
		return
		{
			LOAD_PROJECT_RES::OK,
			Toad::format_str("Loaded project {} successfully", settings.name)
		};
	}

	void LoadProjectResources(const ProjectSettings& project)
	{
		fs::path asset_path = ui::GameAssetsBrowser::FindAssetPath(project);

		for (const auto& entry : fs::directory_iterator(project.project_path))
		{
			if (fs::is_directory(entry.path()))
				continue;

			if (entry.path().extension() == FILE_EXT_FSM)
			{
				Toad::FSM fsm = Toad::FSM::Deserialize(entry.path());
				Toad::ResourceManager::GetFSMs().Add(fs::relative(entry.path(), asset_path).string(), fsm);
			}
		}
	}

	project::PROJECT_TYPE DetectProjectType(const fs::path& proj_dir)
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

	bool Update(const ProjectSettings& settings, const fs::path& path)
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
		int res = system(full_command.c_str());
        
#ifdef __APPLE__
        // try using the full path
        if (res == 32512)
        {
            // for now its just this
            premake5 = "/opt/homebrew/bin/premake5";
            
            // fix the command
#ifdef TOAD_DISTRO
        full_command = Toad::format_str(
            "{} {} --file={} --enginepath={} --projectname={}",
            premake5,
            ProjectTypeAsStr(proj_type),
            (project_file.parent_path() / "premake5.lua").string(),
            settings.engine_path,
            project_name);
#else
       full_command = Toad::format_str(
            "{} {} --file={} --enginepath={} --projectname={} --usesrc",
            premake5,
            ProjectTypeAsStr(proj_type),
            (project_file.parent_path() / "premake5.lua").string(),
            settings.engine_path,
            project_name);
#endif
            int res = system(full_command.c_str());
            LOGDEBUGF("result: {}", res);
            if (res != 0)
            {
                LOGDEBUGF("Failed to execute command {}", full_command);
            }
        }
#endif
		return true;
	}

	std::vector<ProjectTemplate> GetAvailableTemplates(const fs::path& engine_path)
	{
		std::vector<ProjectTemplate> res;

#ifndef TOAD_DISTRO
		fs::path templates_path = engine_path / "GameTemplates";
#else 
		fs::path templates_path = engine_path / "game_templates";
#endif 

		if (!fs::exists(templates_path))
		{
			LOGONCEERROR("[Project] No GameTemplates folder found in engine");
			return res;
		}

		for (const auto& entry : fs::directory_iterator(templates_path))
		{
			if (!entry.is_directory())
				continue;

			if (!fs::exists(entry.path() / "src" / "assets"))
				continue; 
			if (!fs::exists(entry.path() / "src" / "game_core"))
				continue;

			ProjectTemplate pt;
			pt.name = entry.path().filename().string();
			pt.path = entry.path();

			for (const auto& project_entry : fs::directory_iterator(entry))
			{
				if (project_entry.path().extension() == ".png" || project_entry.path().extension() == ".jpg")
				{
					pt.image_preview_loaded = pt.image_preview.loadFromFile(project_entry.path().string());
					continue;
				}

				std::string name = project_entry.path().filename().string();

				for (auto& c : name)
					c = (char)std::tolower(c);

				if (name.find("readme") != std::string::npos)
				{
					std::ifstream readme(project_entry.path());
					if (readme)
					{
						std::stringstream ss;
						ss << readme.rdbuf();
						pt.description = std::move(ss.str());

						// save first 3 lines
						size_t line_3 = 0;
						int counter = 0;
						for (size_t i = 0; i < pt.description.size(); i++)
						{
							if (pt.description[i] == '\n')
							{
								line_3 = i;
								counter++;
								if (counter == 3)
									break;
							}
						}
						pt.description = pt.description.substr(0, line_3);
						readme.close();
					}
				}
			}

			res.emplace_back(pt);
		}

		return res;
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
		if (!fs::is_directory(path / "bin"))
		{
			ri = CREATE_PROJECT_RES_INFO{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have libs/ folder in parent : {}", path)
			};

			return false;
		}
		if (!fs::is_directory(path / "libs"))
		{
			ri = CREATE_PROJECT_RES_INFO{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have libs/ folder in parent : {}", path)
			};

			return false;
		}
		if (!fs::is_directory(path / "script_api"))
		{
			ri = CREATE_PROJECT_RES_INFO{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have script_api/ folder in parent : {}", path)
			};

			return false;
		}
		if (!fs::is_directory(path / "game_templates"))
		{
			ri = CREATE_PROJECT_RES_INFO{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have game_template/ folder in parent : {}", path)
			};

			return false;
		}
		if (!fs::is_directory(path / "scripts"))
		{
			ri = CREATE_PROJECT_RES_INFO{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have scripts/ folder in parent : {}", path)
			};

		}
#else
		// check for runner and vendor (and engine/src/pch.h?) 
		if (!fs::is_directory(path / "ToadRunner"))
		{
			ri = CREATE_PROJECT_RES_INFO{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have ToadRunner/ folder in parent : {}", path)
			};

			return false;
		}
		if (!fs::is_directory(path / "vendor"))
		{
			ri = CREATE_PROJECT_RES_INFO{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Invalid engine path, doesn't have vendor/ folder in parent : {}", path)
			};

			return false;
		}
		std::string bin_output = "Release-" + std::string(PLATFORM_AS_STRING) + "-x86_64";
		std::string lib_file = LIB_FILE_PREFIX + std::string("Engine") + LIB_FILE_EXT;
		if (!fs::exists(path / "bin" / bin_output / lib_file))
		{
			LOGWARNF("[Project] No lib files build in Release, which is needed for project. Please build the engine in Release : {} Doesn't exist", path / "bin" / bin_output / lib_file);
		}
#endif
		return true;
	}

}
