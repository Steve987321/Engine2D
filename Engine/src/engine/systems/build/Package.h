#pragma once

#include "EngineCore.h"

namespace Toad
{

	// only for Windows Visual Studio projects 
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
		bool is_debug = false;
	};

	// bool CreateDebugPackage() or PackageFlags & IsDebugBuild
	bool CreatePackage(const CreatePackageParams& params);

private:
	bool RunBuildSystemWithArgs(const std::filesystem::path& build_program, std::string_view args);
};

}

