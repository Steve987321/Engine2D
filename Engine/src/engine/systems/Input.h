#pragma once

#include "EngineCore.h"

// interface for scripts to handle inputs 

namespace Toad
{

namespace Input
{
	using FKEY_CALLBACK = std::function<void(sf::Keyboard::Key)>;
	using FMOUSE_CALLBACK = std::function<void(sf::Mouse::Button)>;

	ENGINE_API bool IsKeyDown(sf::Keyboard::Key key);
	ENGINE_API bool IsMouseDown(sf::Mouse::Button button);

	ENGINE_API void AddKeyPressCallback(FKEY_CALLBACK callback);
	ENGINE_API void AddMousePressCallback(FMOUSE_CALLBACK callback);
	ENGINE_API void AddKeyReleaseCallback(FKEY_CALLBACK callback);
	ENGINE_API void AddMouseReleaseCallback(FMOUSE_CALLBACK callback);

	ENGINE_API void Clear();

	ENGINE_API void InvokeKeyPressCallbacks(sf::Keyboard::Key key);
	ENGINE_API void InvokeMousePressCallbacks(sf::Mouse::Button button);
	ENGINE_API void InvokeKeyReleaseCallbacks(sf::Keyboard::Key key);
	ENGINE_API void InvokeMouseReleaseCallbacks(sf::Mouse::Button button);

	ENGINE_API const std::vector<FKEY_CALLBACK>& GetAllKeyPressCallbacks();
	ENGINE_API const std::vector<FKEY_CALLBACK>& GetAllKeyReleaseCallbacks();
	ENGINE_API const std::vector<FMOUSE_CALLBACK>& GetAllMousePressCallbacks();
	ENGINE_API const std::vector<FMOUSE_CALLBACK>& GetAllMouseReleaseCallbacks();
}

}