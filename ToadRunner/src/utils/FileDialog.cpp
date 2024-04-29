#include "pch.h"
#include "FileDialog.h"

#ifdef _WIN32
#include <shlobj.h>
#endif

namespace Toad {

	namespace fs = std::filesystem;

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

	std::vector<std::string> GetPathFiles(std::string_view path, std::string_view file_types)
	{
#ifdef _WIN32
		OPENFILENAMEA ofn = { 0 };
		char selected_file[MAX_PATH]{};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = selected_file;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = file_types.data();

		DWORD f = 0;
		f |= OFN_ALLOWMULTISELECT;
		f |= OFN_EXPLORER; // modern style 
		ofn.Flags = f;

		ofn.lpstrInitialDir = fs::path(path).string().c_str();

		std::vector <std::string> files{};

		if (!GetOpenFileNameA(&ofn))
            return files;
        
        // on modern style everything is null seperated 
		if (*ofn.lpstrFile == '\0')
            return files;
        
        const char* p = ofn.lpstrFile;
        std::string s;
        do 
		{
			if (*(p + 1) != '\0')
			{
				s += *p++;
			}
			else
			{
                s += *(p);
				files.emplace_back(s);
				s.clear();
                p += 2;
			}
        } while (*(p) != '\0');

		return files;
#else
        // #TODO: TEST
		return OpenMultipleFilesDialogMac();
#endif
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

        //DWORD f = 0;
        //f |= (bool)(flags & FileDialogFlags::ALLOW_MULTIPLE_SELECTION) ? OFN_ALLOWMULTISELECT : 0;
        //f |= OFN_EXPLORER; // better style 
        //ofn.Flags = f;

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