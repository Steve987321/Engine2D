#pragma once

#include <filesystem>

namespace misc
{

struct Editor {
	std::filesystem::path path;
	std::string name;
};

inline Editor current_editor;

std::vector<Editor> FindEditors();

}
