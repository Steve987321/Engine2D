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
            std::stringstream ss;
            ss << it;


            if (it.is_directory())
            {
                if (ImGui::TreeNode(ss.str().c_str()))
                {
                    IterateDir(fs::directory_iterator(it));
                    ImGui::TreePop();
                }
            }
            else
            {
                if (ImGui::Selectable(ss.str().c_str(), m_selected_file == ss.str()))
                {
                    m_selected_file = ss.str();
                }
            }
        }
    }

    void FileBrowser::Show() {
        ImGui::Text(m_curr_path.c_str());

        ImGui::SameLine();
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

} // Toad