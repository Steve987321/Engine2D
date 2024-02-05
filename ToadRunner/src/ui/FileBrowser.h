#pragma once

#include <filesystem>
#ifdef _WIN32
#include <shlobj.h>
#endif

namespace Toad {

    namespace fs = std::filesystem;

    class FileBrowser {
    public:
        explicit FileBrowser(std::string_view starting_directory);
        ~FileBrowser();

        void Show();
        std::string& GetSelectedFileContent();

        std::string& GetSelectedFile();

        void SetPath(std::string_view path);
        const std::string& GetPath() const;

        bool IsDoubleClicked();
    private:
        void IterateDir(const fs::directory_iterator& dir_it);

        std::vector<std::string> SplitPath(std::string_view path);

        std::string m_selectedFileBuffer;
        std::string m_currPath;
        std::string m_selectedFile;

        bool m_isDoubleClicked = false;
    };

    std::string GetPathDialog(std::string_view title, std::string_view path);
    std::string GetPathFile(std::string_view path, std::string_view file_types);

} // Toad
