#pragma once

namespace Toad {
    std::string GetPathDialog(std::string_view title, std::string_view path);
    std::string GetPathFile(std::string_view path, std::string_view file_types);

#ifdef __APPLE__
    std::string OpenFileDialogMac();
    std::string OpenFolderDialogMac();
#endif
} // Toad