#pragma once

// SFML
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"

#ifdef _WIN32
// api
#ifdef ENGINE_IS_EXPORT
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif
#else
#ifdef ENGINE_IS_EXPORT
#define ENGINE_API 
#else
#define ENGINE_API 
#endif
#endif

#if !defined(TOAD_EDITOR) && defined(NDEBUG) 
#define TOAD_NO_CONSOLE_LOG
#endif 

#ifdef _MSC_VER
#pragma warning (disable: 4996)     // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

#define FILE_EXT_TOADPROJECT ".TOADPROJECT"
#define FILE_EXT_TOADSCENE ".TSCENE"
#ifdef NDEBUG
#define PROJECT_BIN_SEARCH_FOR "Dev"
#else
#define PROJECT_BIN_SEARCH_FOR "DevDebug"
#endif 
#ifdef __APPLE__
#include <sys/syslimits.h>
#include <dlfcn.h>
#include <mach-o/dyld.h> // _NSGetExecutablePath
#define LIB_FILE_PREFIX "lib"
#define LIB_FILE_EXT ".dylib"
#define PATH_SEPARATOR '/'
#define MAX_PATH PATH_MAX
#define PLATFORM_AS_STRING "macosx"
#elif _WIN32
#define PATH_SEPARATOR '\\'
#define LIB_FILE_EXT ".dll"
#define LIB_FILE_PREFIX ""
#define PLATFORM_AS_STRING "windows"
#endif
