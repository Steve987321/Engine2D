#pragma once

#ifdef ERROR 
#undef ERROR 
#endif 

namespace project {

	enum class CREATE_PROJECT_RES
	{
		OK,
		PATH_NOT_EXIST,
		INVALID_PROJECT_SETTINGS,
		ERROR
	};

	struct CREATE_PROJECT_RES_INFO
	{
		CREATE_PROJECT_RES res; 
		std::string description;
	};

	enum class PROJECT_FLAGS{
		NO_VENDOR_COPY = 1 << 0,		// Copies only specified files from vendor to project
		NO_DEFAULT_SCENE = 1 << 1,		// Doesn't copy default game scene to project, blank
	};
	DEFINE_ENUM_FLAG_OPERATORS(PROJECT_FLAGS)

	enum class LOAD_PROJECT_RES
	{
		DOESNT_EXIST
	};

	struct ProjectSettings
	{
		std::string name;
		std::string project_path;
		std::string engine_path;
		PROJECT_FLAGS project_flags;
	};

	// active project 
	inline ProjectSettings current_project{};

	CREATE_PROJECT_RES_INFO Create(const ProjectSettings& settings);

	LOAD_PROJECT_RES Load(const std::string_view path);
}