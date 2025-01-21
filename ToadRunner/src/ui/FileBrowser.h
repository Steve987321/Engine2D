#pragma once

#include <filesystem>

namespace Toad
{

namespace fs = std::filesystem;

class FileBrowser 
{
public:
    explicit FileBrowser(std::string_view starting_directory);
    ~FileBrowser();

    void Show();
    std::string& GetSelectedFileContent();

    std::string& GetSelectedFile();

    void SetPath(const std::filesystem::path& path);
    const std::filesystem::path& GetPath() const;

    void Refresh();

    bool IsDoubleClicked();

private:
    void IterateDir(const fs::directory_iterator& dir_it);

    std::vector<std::string> SplitPath(const std::filesystem::path& path);

    std::string m_selectedFileBuffer;
    std::filesystem::path m_currPath;
    std::string m_selectedFile;

    std::vector<std::filesystem::path> m_pathContents;

    bool m_isDoubleClicked = false;
};

} // Toad
