#include "pch.h"
#include "misc.h"

#include <filesystem>

namespace misc
{

namespace fs = std::filesystem;

std::vector<Editor> misc::FindEditors()
{
	std::vector<Editor> available_editors;

	fs::path vs2022_path = "C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE";
	
	if (fs::exists(vs2022_path))
	{
		available_editors.push_back({ (vs2022_path / "devenv.exe").string(), "Visual Studio 2022" });
	}

	// TODO: 

	return available_editors;
	
}

}