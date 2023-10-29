#include "pch.h"
#include "CreateProject.h"

#include "engine/FormatStr.h"

#include <filesystem>

namespace create_project {


	namespace fs = std::filesystem;

	CREATE_PROJECT_RES_INFO Create(const ProjectSettings& settings)
	{
		std::string game_path = settings.project_path + PATH_SEPARATOR + "Game";
		std::string runner_path = settings.project_path + PATH_SEPARATOR + "ToadRunner";

		if (!fs::is_directory(settings.project_path))
			return {
			CREATE_PROJECT_RES::PATH_NOT_EXIST,
			Toad::format_str("directory {} doesn't exist", settings.project_path) 
		};

		// Create dirs		
		fs::create_directory(game_path);

	}

}