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