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
#ifdef _DEBUG
#define PROJECT_BIN_SEARCH_FOR "DevDebug"
#else
#define PROJECT_BIN_SEARCH_FOR "Dev"
#endif

#ifdef __APPLE__
#include <sys/syslimits.h>
#include <dlfcn.h>
#define LIB_FILE_EXT ".dylib"
#define PATH_SEPARATOR '/'
#define MAX_PATH PATH_MAX
#elif _WIN32
#define PATH_SEPARATOR '\\'
#define LIB_FILE_EXT ".dll"
#endif
