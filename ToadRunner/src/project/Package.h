#pragma once

#include "EngineCore.h"

namespace project
{
    struct ProjectSettings;
}

namespace Toad
{

class Package
{
public:
	ENGINE_API Package();
	ENGINE_API ~Package();

    enum class BuildConfig
    {
        RELEASE,
        DEBUG,
        TEST,
        _count,
    };

	struct CreatePackageParams {
		std::filesystem::path project_file_path;
		std::filesystem::path output_dir_path;
		std::filesystem::path build_system_file_path;
		std::filesystem::path engine_path;
		BuildConfig build_cfg;
	};
	
    ENGINE_API static const char* GetBuildConfigArg(const BuildConfig& cfg);

	// When distributing on mac make sure the required Frameworks are also available on the users computer 
	// manually add frameworks to the rpath Library/Frameworks or ../Frameworks
	ENGINE_API bool CreatePackage(const project::ProjectSettings& settings, const CreatePackageParams& params);

private:
	bool RunBuildSystemWithArgs(const std::filesystem::path& build_program, std::string_view args);
};

}

