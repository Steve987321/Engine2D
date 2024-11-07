#include "pch.h"
#include "Input.h"

namespace Toad
{

namespace Input
{
	static std::vector<FKEY_CALLBACK> key_press_callbacks{};
	static std::vector<FMOUSE_CALLBACK> mouse_press_callbacks{};
	static std::vector<FKEY_CALLBACK> key_up_callbacks{};
	static std::vector<FMOUSE_CALLBACK> mouse_up_callbacks{};

	bool IsKeyDown(sf::Keyboard::Key key)
	{
		return sf::Keyboard::isKeyPressed(key);
	}

	bool IsMouseDown(sf::Mouse::Button button)
	{
		return sf::Mouse::isButtonPressed(button);
	}

	void AddKeyPressCallback(FKEY_CALLBACK callback)
	{
		key_press_callbacks.emplace_back(callback);
	}

	void AddMousePressCallback(FMOUSE_CALLBACK callback)
	{
		mouse_press_callbacks.emplace_back(callback);
	}

	void AddKeyReleaseCallback(FKEY_CALLBACK callback)
	{
		key_up_callbacks.emplace_back(callback);
	}

	void AddMouseReleaseCallback(FMOUSE_CALLBACK callback)
	{
		mouse_up_callbacks.emplace_back(callback);
	}

	void Clear()
	{
		key_press_callbacks.clear();
		key_up_callbacks.clear();
		mouse_press_callbacks.clear();
		mouse_up_callbacks.clear();
	}

	void InvokeKeyPressCallbacks(sf::Keyboard::Key key)
	{
		for (const auto& c : key_press_callbacks)
		{
			if (c)
				c(key);
		}
	}

	void InvokeMousePressCallbacks(sf::Mouse::Button button)
	{
		for (const auto& c : mouse_press_callbacks)
		{
			if (c)
				c(button);
		}
	}

	void InvokeKeyReleaseCallbacks(sf::Keyboard::Key key)
	{
		for (const auto& c : key_up_callbacks)
		{
			if (c)
				c(key);
		}
	}

	void InvokeMouseReleaseCallbacks(sf::Mouse::Button button)
	{
		for (const auto& c : mouse_up_callbacks)
		{
			if (c)
				c(button);
		}
	}

	const std::vector<FKEY_CALLBACK>& GetAllKeyPressCallbacks()
	{
		return key_press_callbacks;
	}

	const std::vector<FKEY_CALLBACK>& GetAllKeyReleaseCallbacks()
	{
		return key_up_callbacks;
	}

	const std::vector<FMOUSE_CALLBACK>& GetAllMousePressCallbacks()
	{
		return mouse_press_callbacks;
	}

	const std::vector<FMOUSE_CALLBACK>& GetAllMouseReleaseCallbacks()
	{
		return mouse_up_callbacks;
	}

}

}