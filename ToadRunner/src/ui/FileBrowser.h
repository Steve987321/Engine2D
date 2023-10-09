#pragma once

#include <filesystem>

namespace Toad {

    namespace fs = std::filesystem;

    class FileBrowser {
    public:
        explicit FileBrowser(std::string_view starting_directory);
        ~FileBrowser();

        void Show();
        std::string& GetSelectedFile();
    private:
        void IterateDir(const fs::directory_iterator& recursiveIt);

        std::vector<std::string> SplitPath(std::string_view path);

        std::string m_curr_path;
        std::string m_selected_file;
    };

} // Toad
