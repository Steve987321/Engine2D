#pragma once

#include "EngineCore.h"

namespace Toad
{

class ENGINE_API Package
{
public:
	Package();
	~Package();

	struct CreatePackageParams {
		std::filesystem::path project_file_path;
		std::filesystem::path output_dir_path;
		std::filesystem::path build_system_file_path;
		std::filesystem::path engine_path;
		bool is_debug = false; // whether to use debug configurations when creating a build 
	};
	
	// When distributing on mac make sure the required Frameworks are also available on the users computer 
	// manually add frameworks to the rpath Library/Frameworks or ../Frameworks
	bool CreatePackage(const CreatePackageParams& params);

private:
	bool RunBuildSystemWithArgs(const std::filesystem::path& build_program, std::string_view args);
};

}

