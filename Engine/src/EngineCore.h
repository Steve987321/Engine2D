#pragma once

// SFML
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

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

#ifdef _MSC_VER
#pragma warning (disable: 4996)     // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

#ifdef __APPLE__
#include <sys/syslimits.h>
#define PATH_SEPARATOR '/'
#define MAX_PATH PATH_MAX
#else
#define PATH_SEPARATOR '\\'
#endif