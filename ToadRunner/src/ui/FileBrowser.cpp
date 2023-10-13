#include "pch.h"

#include "Engine/Engine.h"

#include "FileBrowser.h"

#include <sstream>


namespace Toad {

    FileBrowser::FileBrowser(std::string_view starting_directory) {
        m_curr_path = starting_directory;
    }

    FileBrowser::~FileBrowser() {

    }

    void FileBrowser::IterateDir(const fs::directory_iterator& dir_it)
    {
        for (const auto& it : dir_it)
        {
            auto labelStr = it.path().filename().string();
            if (it.is_directory())
                labelStr += PATH_SEPARATOR;

            auto full = it.path().string();
            if (ImGui::Selectable(labelStr.c_str(), m_selected_file == full, ImGuiSelectableFlags_AllowDoubleClick))
            {
                // read it
                std::ifstream f;
                f.open(full);
                if (f.is_open())
                {
                    std::stringstream ss;
                    ss << f.rdbuf();
                    m_selected_file_buffer = ss.str();
                    f.close();
                }

                m_selected_file = full;

                if (ImGui::IsMouseDoubleClicked(0))
                {
                    if (it.is_directory())
                    {
                        m_curr_path = full;
                    }
                }
            }
        }
    }

    void FileBrowser::Show() {
        auto folders = SplitPath(m_curr_path);

        for (int i = 0; i < folders.size(); i++)
        {
            auto& folder = folders[i];

            if (i <= folders.size() - 1)
                ImGui::SameLine();

            if (ImGui::Button((folder + PATH_SEPARATOR).c_str()))
            {
                if (i == folders.size() - 1)
                    break;

                std::string update_path;
#ifndef _WIN32
                update_path += PATH_SEPARATOR;
#endif
                for (int j = 0; j <= i; j++)
                {
                    update_path += folders[j];
                    if (j < i)
                        update_path += PATH_SEPARATOR;
                }
                m_curr_path = update_path;
            }
        }

        IterateDir(fs::directory_iterator(m_curr_path));
    }

    std::string &FileBrowser::GetSelectedFileContent() {
        return m_selected_file_buffer;
    }

    std::string& FileBrowser::GetSelectedFile() {
        return m_selected_file;
    }

    std::vector<std::string> FileBrowser::SplitPath(std::string_view path) {
        std::string folder;
        std::vector<std::string> res;
        for (auto c : path)
        {
            if (c == PATH_SEPARATOR) {
                if (folder.empty())
                    continue;

                res.push_back(folder);
                folder.clear();
            }
            else
            {
                folder += c;
            }
        }

        res.push_back(folder);
        return res;
    }

} // Toad