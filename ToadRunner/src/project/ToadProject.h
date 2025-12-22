#pragma once

#ifdef ERROR 
#undef ERROR 
#endif 

#include "Misc.h"
#include "EngineCore.h"
#include "engine/Types.h"
#include "engine/utils/Helpers.h"
#include "nlohmann/json.hpp"

namespace misc
{
	struct Editor;
}

namespace project {

	enum class CREATE_PROJECT_RES
	{
		OK,
		PATH_NOT_EXIST,
		INVALID_PROJECT_SETTINGS,
		ERROR,
        _count,
	};

	inline std::ostream& operator<<(std::ostream& os, CREATE_PROJECT_RES& r)
	{
		switch(r){
		case CREATE_PROJECT_RES::OK:
			os << "OK";
			break;
		case CREATE_PROJECT_RES::PATH_NOT_EXIST: 
			os << "PATH_NOT_EXIST";
			break;
		case CREATE_PROJECT_RES::INVALID_PROJECT_SETTINGS:
			os << "INVALID_PROJECT_SETTINGS";
			break;
		case CREATE_PROJECT_RES::ERROR:
			os << "ERROR";
			break;
        default:
            break;
		}

		return os;
	}

	struct CREATE_PROJECT_RES_INFO
	{
		CREATE_PROJECT_RES res; 
		std::string description;
	};

	enum class PROJECT_FLAGS{
		NO_VENDOR_COPY = 1 << 0,		// Copies only specified files from vendor to project
		NO_DEFAULT_SCENE = 1 << 1,		// Doesn't copy default game scene to project, blank
        NO_DEFAULT_SCRIPTS = 1 << 1,	// Doesn't copy default game scripts to project
	};
	DEFINE_ENUM_FLAG_OPERATORS(PROJECT_FLAGS)

    enum class PROJECT_TYPE{
        VS_2022,
        VS_2019,
        VS_2015,
        Makefile,
        CMake,
        Codelite,
        Xcode,
    };

    struct LOAD_PROJECT_RES_INFO;

	struct ProjectSettings
	{
		std::string name;
		std::filesystem::path project_path;
		std::filesystem::path engine_path;
		PROJECT_FLAGS project_flags = PROJECT_FLAGS::NO_VENDOR_COPY;
		PROJECT_TYPE project_gen_type = PROJECT_TYPE::Makefile;
		Toad::Vec2f editor_cam_size = {1280, 720};
		Toad::Vec2f editor_cam_pos = {0, 0};
        bool use_own_libs = false;

		nlohmann::json ToJSON() const;
        LOAD_PROJECT_RES_INFO Deserialize(const std::filesystem::path& file);
	};


	enum class LOAD_PROJECT_RES
	{
		OK,
		DOESNT_EXIST,
        FAILED_TO_OPEN,
		INVALID_PROJECT_FILE,
	};

    struct LOAD_PROJECT_RES_INFO 
	{
		LOAD_PROJECT_RES res;
		std::string description;
		ProjectSettings settings;
	};

	// little information about the project template
	struct ProjectTemplate
	{
		std::string name;

		// optional could be empty
		std::string description;

		// full path
		std::filesystem::path path;

		sf::Texture image_preview;

		bool image_preview_loaded = false;
	};

	// active project 
	inline ProjectSettings current_project{};

#ifdef _WIN32
	bool OpenSln(const std::filesystem::path& settings, const misc::Editor& editor = misc::current_editor);
#endif

	// very minimal checking 
	bool VerifyEnginePathContents(const std::filesystem::path& path, CREATE_PROJECT_RES_INFO& ri);

    std::string ProjectTypeAsStr(PROJECT_TYPE r);

	CREATE_PROJECT_RES_INFO Create(const ProjectSettings& settings, const std::string& selected_template);

	LOAD_PROJECT_RES_INFO Load(const std::filesystem::path& path);

	void LoadProjectResources(const std::filesystem::path& project_path);

	PROJECT_TYPE DetectProjectType(const std::filesystem::path& proj_dir);

	// rerun premake path=projectpath
    // if there are problems it will edit settings to fix them 
	bool Update(ProjectSettings& settings, bool detect_proj_type = true);

    bool ResetPremakeFile(const ProjectSettings& settings);

	std::vector<ProjectTemplate> GetAvailableTemplates(const std::filesystem::path& path);
}
