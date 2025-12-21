#include "pch.h"
#include "ToadProject.h"

#include "engine/Engine.h"
#include "Misc.h"

#include "ui/GameAssetsBrowser.h"
#include "ui/MessageQueue.h"

#include "nlohmann/json.hpp"

#include <filesystem>
#include <fstream>

namespace project {

	namespace fs = std::filesystem;
	using json = nlohmann::ordered_json;

    struct PremakeCmdArgs
    {
        std::string_view    premake;
        PROJECT_TYPE        proj_type; 
        fs::path            proj_file;
        std::string_view    proj_name;
        fs::path            engine_path;
        fs::path            script_dir;
        bool                use_src;
        bool                use_own_libs;
    };

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

    static bool VerifySettingsPaths(const ProjectSettings& settings, CREATE_PROJECT_RES_INFO& ri)
    {
		if (!fs::is_directory(settings.project_path))
		{
			ri = 
			{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Directory {} doesn't exist for project", settings.project_path)
			};

            return false;
		}

		if (!fs::is_empty(settings.project_path))
		{
#ifdef _WIN32
            // change to warning, choose to add/overwrite directory contents
            ri = 
            {
                CREATE_PROJECT_RES::ERROR,
                Toad::format_str("Project directory isn't empty {}", settings.project_path)
            };

			return false;
#else
            // on mac ignore .DS_STORE
			for (const auto& entry : fs::directory_iterator(settings.project_path))
			{
				if (entry.is_directory())
				{
					// change to warning, choose to add/overwrite directory contents
					ri =
					{
						CREATE_PROJECT_RES::ERROR,
						Toad::format_str("Project directory isn't empty {}", settings.project_path)
					};
                    return false;
				}
			}
#endif
		}
		
        if (!fs::is_directory(settings.engine_path))
		{
			ri = 
			{
				CREATE_PROJECT_RES::PATH_NOT_EXIST,
				Toad::format_str("Directory {} doesn't exist for engine path", settings.engine_path)
			};

            return false;
		}

        return true; 
    }

    static bool EnsureCmakeGenScripts(const fs::path& proj_path, const fs::path& engine_path, fs::path& scripts_dir)
    {
        if (fs::exists(proj_path / "scripts" / "cmake"))
            scripts_dir = proj_path;
        else if (fs::exists(engine_path / "scripts" / "cmake"))
            scripts_dir = engine_path;
        else 
            return false;

        // check require "scripts/cmake" line in premake5.lua

        fs::path premake5_file_path = proj_path / "premake5.lua";

        if (!fs::exists(premake5_file_path))
            return false;
        
        std::ifstream premake5_file(premake5_file_path);

        if (!premake5_file)
            return false; 

        std::string original_content((std::istreambuf_iterator<char>(premake5_file)), std::istreambuf_iterator<char>());

        premake5_file.close();

        std::string cmake_require_line = "require \"scripts/cmake\"";

        if (original_content.find(cmake_require_line) != std::string::npos)
            return true; 

        std::string new_content = cmake_require_line + "\n\n" + original_content;

        std::ofstream out_file(premake5_file_path, std::ios::trunc);
        if (!out_file)
            return false;

        out_file << new_content;

        return true;
    }

    // args will be modified depending on failure of running command 
    // fail_count is used for failure on non windows systems, don't use 
    static bool DoPremakeCommand(PremakeCmdArgs& args, int fail_count = 0)
    {
        std::string full_command = Toad::format_str("{} {} --file={} --enginepath={} --projectname={}",
            args.premake,
			ProjectTypeAsStr(args.proj_type),
			(args.proj_file.parent_path() / "premake5.lua").string(),
			args.engine_path,
			args.proj_name);

        if (!args.script_dir.empty())
            full_command += Toad::format_str(" --scripts={}", args.script_dir);      

        if (args.use_src)
            full_command += " --usesrc";

        if (args.use_own_libs)
            full_command += " --ownlibs";

		LOGDEBUGF("Running command: {}", full_command);
		int res = system(full_command.c_str());
        
        LOGDEBUGF("Command res: {}", res);

#ifdef __APPLE__
        // try using the full path
        if (res == 32512 && fail_count < 3)
        {
            // for now its just this
            args.premake = "/opt/homebrew/bin/premake5";
            
            return DoPremakeCommand(args, fail_count++);
        }
#endif 

        return res == 0; 
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

	static bool VerifyEnginePathContents(const fs::path& path, CREATE_PROJECT_RES_INFO& ri)
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

	CREATE_PROJECT_RES_INFO Create(const ProjectSettings& settings, const std::string& selected_template)
	{
		std::string project_path_forwardslash = settings.project_path.string();
		std::string engine_path_forwardslash = settings.engine_path.string();

		path_as_forward_slash(engine_path_forwardslash);
		path_as_forward_slash(project_path_forwardslash);

        const fs::path& engine_path_fs = settings.engine_path;
		const fs::path generate_game_lua = engine_path_fs / "scripts" / "generate_game_project.lua";

		// #TODO: CHANGE 
#ifdef _WIN32
		const std::string premake5 = "premake5.exe";
		const std::string full_path_to_premake5 = project_path_forwardslash + '/' + premake5;
#else
		std::string premake5 = "premake5";
		const std::string& full_path_to_premake5 = premake5;
#endif
	
#ifdef TOAD_DISTRO
		const fs::path bin_path = engine_path_fs.string() + "/bin/" + premake5;
#else
		const fs::path bin_path = engine_path_fs.string() + "/vendor/bin/" + premake5;
#endif

		CREATE_PROJECT_RES_INFO ri;

        if (!VerifySettingsPaths(settings, ri))
            return ri; 

		if (!VerifyEnginePathContents(settings.engine_path, ri))
			return ri;

        // copy project premake file 
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

		// create/copy default game/engine files 

		std::string game_path = project_path_forwardslash + "/" + settings.name + "_Game/src";

        try
		{
			fs::create_directories(game_path);
		}
		catch (fs::filesystem_error e)
		{
			std::cout << e.what() << std::endl;
		}

        if (settings.use_own_libs)
        {
#ifdef TOAD_DISTRO
		fs::copy(engine_path_fs / "game_templates" / selected_template / "src", game_path, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
		fs::copy(engine_path_fs / "game_templates" / "vendor", project_path_forwardslash + "/vendor", fs::copy_options::overwrite_existing | fs::copy_options::recursive);
#else
		fs::copy(engine_path_fs / "GameTemplates" / selected_template / "src", game_path, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
		for (const auto& e : fs::directory_iterator(engine_path_fs / "vendor"))
			if (e.path().filename().string().find("imgui") != std::string::npos ||
				e.path().filename().string().find("json") != std::string::npos ||
				e.path().filename().string().find("SFML-3") != std::string::npos ||
				e.path().filename().string().find("magic_enum") != std::string::npos ||
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
#endif 
        }

		// project file 
        fs::path proj_file_path = fs::path(settings.project_path) / (settings.name + FILE_EXT_TOADPROJECT);
		std::ofstream proj_file(proj_file_path);

		if (proj_file.is_open())
		{
            json data = settings.ToJSON();

			proj_file << std::setw(4) << data;
			proj_file.close();
		}

        // generate project files after adding sources 

        fs::path scripts_dir_arg;
        if (settings.project_gen_type == PROJECT_TYPE::CMake)
        {
            if (!EnsureCmakeGenScripts(settings.project_path, settings.engine_path, scripts_dir_arg))
                return {
                    CREATE_PROJECT_RES::ERROR,
                    "Failed to verify cmake generator scripts"
                };
        }

        PremakeCmdArgs args;
        args.premake = premake5;
        args.proj_type = settings.project_gen_type;
        args.proj_file = proj_file_path;
        args.proj_name = settings.name;
        args.engine_path = settings.engine_path;
        args.script_dir = scripts_dir_arg;
        args.use_own_libs = settings.use_own_libs;

#ifndef TOAD_DISTRO 
        args.use_src = true; 
#endif 
        DoPremakeCommand(args);

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
        }

        return
        {
            CREATE_PROJECT_RES::OK,
            Toad::format_str("Created project {}", settings.name)
        };
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

			auto ri = settings.Deserialize(path);
            if (ri.res != LOAD_PROJECT_RES::OK)
            {
                return ri;
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
        Toad::ResourceManager::Init();

        Toad::DrawingCanvas::ClearDrawBuffers();
        for (auto& canvas : Toad::DrawingCanvas::GetCanvases())
            canvas->ClearVertices();

		current_project = settings;
		Toad::SetProjectPath(current_project.project_path);
		
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

    bool Update(const ProjectSettings& settings, bool detect_proj_type)
	{
		if (!fs::exists(settings.project_path))
		{
			LOGERRORF("[Project] {} doesn't exist", settings.project_path);
			return false;
		}

		if (!fs::exists(settings.engine_path))
		{
			LOGERRORF("[Project] {} doesn't exist", settings.engine_path);
			return false;
		}

		fs::path project_file = settings.project_path;

		if (fs::is_directory(project_file))
		{
			for (const auto& entry : fs::directory_iterator(project_file))
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
			LOGERRORF("[Project] Can't find project file in {}", settings.project_path);
			return false;
		}

		std::ifstream project_file_contents(project_file);

		if (!project_file_contents)
		{
			LOGERRORF("[Project] Can't read {}", settings.project_path);
			return false;
		}

		json data;
		try {
			data = json::parse(project_file_contents);
		}
		catch (json::parse_error& e) {
			LOGERRORF("[Project] Failed to parse {}, {}", settings.project_path, e.what());
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

#ifdef TOAD_DISTRO
		const std::filesystem::path engine_relative_premake5_exe = "bin\\premake5.exe";
#else 
		const std::filesystem::path engine_relative_premake5_exe = "vendor\\bin\\premake5.exe";
#endif 

		premake5 = (project_file.parent_path() / "premake5.exe").string();
		if (!fs::exists(premake5))
		{
			LOGWARNF("[Project] Can't find premake5.exe: '{}'", premake5);
			premake5 = (settings.engine_path / engine_relative_premake5_exe).string();
			if (!fs::exists(premake5))
			{
				LOGERRORF("[Project] Can't find premake5.exe: '{}'", premake5);
				return false;
			}
		}
#else
		premake5 = "premake5";
#endif

        PROJECT_TYPE proj_type;

        if (detect_proj_type)
		    proj_type = DetectProjectType(project_file.parent_path());
        else 
            proj_type = settings.project_gen_type;

        fs::path scripts_dir_arg;

        if (proj_type == PROJECT_TYPE::CMake)
        {
            if (!EnsureCmakeGenScripts(project_file.parent_path(), settings.engine_path, scripts_dir_arg))
            {
                LOGERROR("[Project] Cmake generator script can't be verified, Continuing updating project files...");

                // detect anyways 
		        proj_type = DetectProjectType(project_file.parent_path());
            }
        }

		// #TODO: should probably add an extra option to generate chosen project type instead of automatically deciding

		// #TODO should it use the (updated) one in the engine directory or the one in the game directory which may be outdated ?
        
        PremakeCmdArgs args;
        args.premake = premake5;
        args.proj_type = proj_type;
        args.proj_file = project_file;
        args.proj_name = project_name;
        args.engine_path = settings.engine_path;
        args.script_dir = scripts_dir_arg;
        args.use_own_libs = settings.use_own_libs;

#ifndef TOAD_DISTRO 
        args.use_src = true; 
#endif 
        return DoPremakeCommand(args);

		return true;
	}

    bool ResetPremakeFile(const ProjectSettings &settings) 
    {
        using namespace Toad; 

        if (!fs::exists(settings.project_path / "premake5.lua"))
        {
            MessageQueueMessage msg;
            msg.category = MessageCategory::OTHER;
            msg.type = MessageType::ERROR;
            msg.msg = Toad::format_str("No premake5.lua found in: '{}'", settings.project_path);
            MessageQueue::AddToMessageQueue(msg);
            return false;
        }

        // #TODO Finsih
        
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
            case PROJECT_TYPE::VS_2019:
                return "vs2019";
            case PROJECT_TYPE::VS_2015:
                return "vs2015";
            case PROJECT_TYPE::Makefile:
                return "gmake";
            case PROJECT_TYPE::Codelite:
                return "codelite";
            case PROJECT_TYPE::Xcode:
                return "xcode4";
            case PROJECT_TYPE::CMake:
                return "cmake";
            default:
                return "invalid";
        }
    }

    nlohmann::json ProjectSettings::ToJSON() const 
    {
        nlohmann::json data;
        data["name"] = name;
        data["editor_cam_sizex"] = editor_cam_size.x;
        data["editor_cam_sizey"] = editor_cam_size.y;	
        data["editor_cam_posx"] = editor_cam_pos.x;
        data["editor_cam_posy"] = editor_cam_pos.y;
        data["use_own_libs"] = use_own_libs;
        return data;
    }

    LOAD_PROJECT_RES_INFO ProjectSettings::Deserialize(const std::filesystem::path &file) 
    {
        std::ifstream project_file(file);

        if (!project_file.is_open())
            return
            {
                LOAD_PROJECT_RES::FAILED_TO_OPEN,
                Toad::format_str("Failed to open project file: {}", file),
                *this
            };

        try
        {
            json data = json::parse(project_file);

            project_path = fs::path(file).string();
            
            GET_JSON_ELEMENT(name, data, "name");
            GET_JSON_ELEMENT(editor_cam_pos.x, data, "editor_cam_posx");
            GET_JSON_ELEMENT(editor_cam_pos.y, data, "editor_cam_posy");
            GET_JSON_ELEMENT(editor_cam_size.x, data, "editor_cam_sizex");
            GET_JSON_ELEMENT(editor_cam_size.y, data, "editor_cam_sizex");		
            GET_JSON_ELEMENT(use_own_libs, data, "use_own_libs");
        }
        catch(json::parse_error& e)
        {
            project_file.close();

            return
            {
                LOAD_PROJECT_RES::INVALID_PROJECT_FILE,
                Toad::format_str("Parse error at {}, {}. While parsing {}", e.byte, e.what(), file),
                *this
            };
        }

        project_file.close();

        return     
        {
            LOAD_PROJECT_RES::OK,
            "OK",
            *this
        };
    }

} 
