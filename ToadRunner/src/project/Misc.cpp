#include "pch.h"
#include "Misc.h"

#include <EngineCore.h>

#include <filesystem>

namespace misc
{

namespace fs = std::filesystem;

std::vector<Editor> FindEditors()
{
#ifdef __APPLE__
    return {};
#endif
	std::vector<Editor> available_editors;

	fs::path vs2022_path = "C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE";
	
	if (fs::exists(vs2022_path))
	{
		available_editors.push_back({ (vs2022_path / "devenv.exe"), "Visual Studio 2022" });
	}

	// TODO: 

	return available_editors;
}

std::filesystem::path GetExePath()
{
	
#ifdef _WIN32
	char path[MAX_PATH];
	GetModuleFileNameA(NULL, path, MAX_PATH);
	return path;
#else
	char path[PATH_MAX];
	uint32_t size = MAX_PATH;
	_NSGetExecutablePath(path, &size);
	return path; 
#endif
}

}