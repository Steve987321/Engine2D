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
		ERROR
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
        Codelite,
        Xcode,
    };

	enum class LOAD_PROJECT_RES
	{
		OK,
		DOESNT_EXIST,
		INVALID_PROJECT_FILE,
	};
	
	struct ProjectSettings
	{
		std::string name;
		std::string project_path;
		std::string engine_path;
		PROJECT_FLAGS project_flags;
		PROJECT_TYPE project_gen_type;
		Vec2f editor_cam_size = {1280, 720};
		Vec2f editor_cam_pos = {0, 0};

		nlohmann::json to_json() const
		{
			nlohmann::json data;
			data["name"] = name;
			data["project_path"] = project_path;
			data["editor_cam_sizex"] = editor_cam_size.x;
			data["editor_cam_sizey"] = editor_cam_size.y;	
			data["editor_cam_posx"] = editor_cam_pos.x;
			data["editor_cam_posy"] = editor_cam_pos.y;
			return data;
		}
	};

	struct LOAD_PROJECT_RES_INFO 
	{
		LOAD_PROJECT_RES res;
		std::string description;
		ProjectSettings settings;
	};

	// active project 
	inline ProjectSettings current_project{};

#ifdef _WIN32
	bool OpenSln(const std::filesystem::path& settings, const misc::Editor& editor = misc::current_editor);
#endif

    std::string ProjectTypeAsStr(PROJECT_TYPE r);

	CREATE_PROJECT_RES_INFO Create(const ProjectSettings& settings);

	LOAD_PROJECT_RES_INFO Load(const std::string_view path);

	PROJECT_TYPE DetectProjectType(const std::filesystem::path& proj_dir);

	// rerun premake path=projectpath
	bool Update(const ProjectSettings& settings, const std::filesystem::path& path);
}
