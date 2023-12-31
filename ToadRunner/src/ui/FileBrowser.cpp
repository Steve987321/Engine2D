#include "pch.h"

#include "Engine/Engine.h"

#include "FileBrowser.h"

namespace Toad {

    extern int CALLBACK ProjectBrowseFolderCallback(HWND hwnd, UINT msg, LPARAM lparam, LPARAM lpdata);

    FileBrowser::FileBrowser(std::string_view starting_directory) {
        m_currPath = starting_directory;
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
            if (ImGui::Selectable(labelStr.c_str(), m_selectedFile == full, ImGuiSelectableFlags_AllowDoubleClick))
            {
                // read it
                std::ifstream f;
                f.open(full);
                if (f.is_open())
                {
                    std::stringstream ss;
                    ss << f.rdbuf();
                    m_selectedFileBuffer = ss.str();
                    f.close();
                }

                m_selectedFile = full;

                if (ImGui::IsMouseDoubleClicked(0))
                {
                    if (it.is_directory())
                    {
                        m_currPath = full;
                    }
                    m_isDoubleClicked = true;
                }
            }

            // TODO : accept  drag and drop from game asset browser 
            if (it.is_directory() && ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("move file"))
                {
                    fs::path src = *(std::string*)payload->Data;
                    std::error_code e;
                    fs::rename(src, it.path() / src.filename(), e);
                    LOGDEBUGF("error code message: {} {}", e.message(), e.value());
                }
            }
        }
    }

    void FileBrowser::Show() {
        m_isDoubleClicked = false;

        auto folders_in_path = SplitPath(m_currPath);

        for (int i = 0; i < folders_in_path.size(); i++)
        {
            auto& folder = folders_in_path[i];

            if (i <= folders_in_path.size() - 1)
                ImGui::SameLine();

            if (ImGui::Button((folder + PATH_SEPARATOR).c_str()))
            {
                if (i == folders_in_path.size() - 1)
                    break;

                std::string update_path;
#ifndef _WIN32
                update_path += PATH_SEPARATOR;
#endif
                for (int j = 0; j <= i; j++)
                {
                    update_path += folders_in_path[j];
                    if (j < i)
                        update_path += PATH_SEPARATOR;
                }
                m_currPath = update_path;
            }
        }

        IterateDir(fs::directory_iterator(m_currPath));
    }

    std::string &FileBrowser::GetSelectedFileContent() {
        return m_selectedFileBuffer;
    }

    std::string& FileBrowser::GetSelectedFile() {
        return m_selectedFile;
    }

    void FileBrowser::SetPath(std::string_view path) {
        if (!fs::is_directory(path))
        {   
            LOGDEBUGF("[FileBrowser] {} isn't a valid directory", path);
            return;
        }

        m_currPath = path;
    }

    const std::string& FileBrowser::GetPath() const
    {
        return m_currPath;
    }

    bool FileBrowser::IsDoubleClicked()
    {
        return m_isDoubleClicked;
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

    std::string GetPathDialog(std::string_view title, std::string_view path)
    {
        BROWSEINFOA bi = { 0 };
        bi.lpszTitle = title.data();
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
        bi.lpfn = ProjectBrowseFolderCallback;
        bi.lParam = (LPARAM)path.data();

        auto item = SHBrowseForFolderA(&bi);

        char selected_path[MAX_PATH]{};

        if (item != NULL)
        {
            SHGetPathFromIDListA(item, selected_path);
            return selected_path;
        }

        return "";
    }

    std::string GetPathFile(std::string_view path, std::string_view file_types)
    {
        OPENFILENAMEA ofn = { 0 };
        char selected_file[MAX_PATH]{};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = selected_file;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = file_types.data();

        // TODO: finish this function
        ofn.lpstrInitialDir = fs::path(path).string().c_str();

        if(!GetOpenFileNameA(&ofn))
        {
            return "";
        }

        return ofn.lpstrFile;
    }

    int ProjectBrowseFolderCallback(HWND hwnd, UINT msg, LPARAM lparam, LPARAM lpdata)
    {
        if (msg == BFFM_INITIALIZED)
        {
            LPCTSTR path = reinterpret_cast<LPCTSTR>(lpdata);
            SendMessage(hwnd, BFFM_SETSELECTION, true, (LPARAM)path);
        }
        return 0;
    }

} // Toad