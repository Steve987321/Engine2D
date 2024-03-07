#pragma once

#include "EngineCore.h"

// interface for scripts to handle inputs 

namespace Toad
{

	class ENGINE_API Input
	{
	public:
		friend class Engine;

		typedef void(*FKEY_CALLBACK)(sf::Keyboard::Key);
		typedef void(*FMOUSE_CALLBACK)(sf::Mouse::Button);

		static bool IsKeyDown(sf::Keyboard::Key key);
		static bool IsMouseDown(sf::Mouse::Button button);

		static void AddKeyPressCallback(FKEY_CALLBACK callback);
		static void AddMousePressCallback(FMOUSE_CALLBACK callback);
		static void AddKeyReleaseCallback(FKEY_CALLBACK callback);
		static void AddMouseReleaseCallback(FMOUSE_CALLBACK callback);

		static void Clear();

	private:
		static void InvokeKeyPressCallbacks(sf::Keyboard::Key key);
		static void InvokeMousePressCallbacks(sf::Mouse::Button button);
		static void InvokeKeyReleaseCallbacks(sf::Keyboard::Key key);
		static void InvokeMouseReleaseCallbacks(sf::Mouse::Button button);

		inline static std::vector<FKEY_CALLBACK> m_keyPressCallbacks{};
		inline static std::vector<FMOUSE_CALLBACK> m_mousePressCallbacks{};
		inline static std::vector<FKEY_CALLBACK> m_keyUpCallbacks{};
		inline static std::vector<FMOUSE_CALLBACK> m_mouseUpCallbacks{};
	};

}