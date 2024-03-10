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

	// bool CreateDebugPackage() or PackageFlags & IsDebugBuild
	bool CreatePackage(const std::filesystem::path& project_path, const std::filesystem::path& output_path, const std::filesystem::path& build_system_path, const std::filesystem::path& engine_path);

private:
	bool RunBuildSystemWithArgs(const std::filesystem::path& build_program, std::string_view args);
};

}

