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

    void FileBrowser::IterateDir(const fs::directory_iterator& recursiveIt)
    {
        for (const auto& it : recursiveIt)
        {
            auto labelStr = it.path().filename().string();
            if (it.is_directory())
                labelStr += '/';

            auto full = it.path().string();
            if (ImGui::Selectable(labelStr.c_str(), m_selected_file == full, ImGuiSelectableFlags_AllowDoubleClick))
            {
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
//        ImGui::Text(m_curr_path.c_str());

        ImGui::SameLine();

        auto folders = SplitPath(m_curr_path);

        for (int i = 0; i < folders.size(); i++)
        {
            auto& folder = folders[i];

            if (ImGui::Button((folder + '/').c_str()))
            {
                if (i == folders.size() - 1)
                {
                    ImGui::SameLine();
                    break;
                }

                std::string update_path = "/";
                for (int j = 0; j < i; j++)
                {
                    update_path += folders[j];

                    if (j < i - 1)
                        update_path += '/';
                }
                m_curr_path = update_path;
            }
            ImGui::SameLine();
        }
        if (ImGui::Button("Back"))
        {
            for (int i = m_curr_path.length(); i > 0; i--)
            {
                if (m_curr_path[i] == '/' )
                {
                    m_curr_path = m_curr_path.substr(0, i);
                    break;
                }
            }
        }

        IterateDir(fs::directory_iterator(m_curr_path));
    }

    std::string& FileBrowser::GetSelectedFile() {
        return m_selected_file;
    }

    std::vector<std::string> FileBrowser::SplitPath(std::string_view path) {
        std::string folder;
        std::vector<std::string> res;
        for (auto c : path)
        {
            if (c == '/') {
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