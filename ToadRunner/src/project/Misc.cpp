#include "pch.h"
#include "misc.h"

#include <filesystem>

namespace misc
{

namespace fs = std::filesystem;

std::vector<Editor> misc::FindEditors()
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
	char path[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, path, MAX_PATH);
	return path;
#else
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	return std::string(result, (count > 0) ? count : 0);
#endif
}

}