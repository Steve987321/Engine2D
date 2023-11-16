#pragma once

namespace misc
{

struct Editor {
	std::string path;
	std::string name;
};

inline Editor current_editor;

std::vector<Editor> FindEditors();

}
