#include "pch.h"
#include "FileDialog.h"

#ifdef _WIN32
#include <shlobj.h>
#endif

namespace Toad {
#ifdef _WIN32
    extern int CALLBACK ProjectBrowseFolderCallback(HWND hwnd, UINT msg, LPARAM lparam, LPARAM lpdata);
#endif

    std::string GetPathDialog(std::string_view title, std::string_view path)
    {
#ifdef _WIN32
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
#else
        return OpenFolderDialogMac();
#endif
        return "";
    }

    std::string GetPathFile(std::string_view path, std::string_view file_types)
    {
#ifdef _WIN32
        OPENFILENAMEA ofn = { 0 };
        char selected_file[MAX_PATH]{};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = selected_file;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = file_types.data();

        ofn.lpstrInitialDir = fs::path(path).string().c_str();

        if(!GetOpenFileNameA(&ofn))
        {
            return "";
        }

        return ofn.lpstrFile;
#else
        return OpenFileDialogMac();
#endif

        return "";
    }

#ifdef _WIN32
    int ProjectBrowseFolderCallback(HWND hwnd, UINT msg, LPARAM lparam, LPARAM lpdata)
    {
        if (msg == BFFM_INITIALIZED)
        {
            LPCTSTR path = reinterpret_cast<LPCTSTR>(lpdata);
            SendMessage(hwnd, BFFM_SETSELECTION, true, (LPARAM)path);
        }
        return 0;
    }
#endif

} // Toad