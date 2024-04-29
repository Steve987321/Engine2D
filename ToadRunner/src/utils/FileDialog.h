#pragma once

namespace Toad {

    // #TODO: expand
    // Flags for dialog window, some won't have any effect depending on system
  //  enum class FileDialogFlags
  //  {
  //      NONE,
		//ALLOW_MULTIPLE_SELECTION = 1 << 2,  // select multiple files/directories
		//CAN_CREATE_DIRECTORIES = 1 << 3,    // only mac 
  //  };
  //  DEFINE_ENUM_FLAG_OPERATORS(FileDialogFlags);

    // Return directory
    std::string GetPathDialog(std::string_view title, std::string_view path);

    // Return multiple files 
    // file_types is only used on Windows
    std::vector<std::string> GetPathFiles(std::string_view path, std::string_view file_types);

    // Return files
    // flags is only used for windows file dialog
	// file_types is only used on Windows
    std::string GetPathFile(std::string_view path, std::string_view file_types);

#ifdef __APPLE__
    std::string OpenFileDialogMac();
    std::vector<std::string> OpenMultipleFilesDialogMac();
    std::string OpenFolderDialogMac();
#endif
} // Toad