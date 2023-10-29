#pragma once

namespace create_project {
	enum class CREATE_PROJECT_RES
	{
		OK,
		PATH_NOT_EXIST,
		INVALID_PROJECT_SETTINGS
	};

	struct CREATE_PROJECT_RES_INFO
	{
		CREATE_PROJECT_RES res; 
		std::string error_description;
	};

	struct ProjectSettings
	{
		std::string name;
		std::string project_path;
	};

	CREATE_PROJECT_RES_INFO Create(const ProjectSettings& settings);
}