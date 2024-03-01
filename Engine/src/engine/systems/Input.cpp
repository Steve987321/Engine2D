#include "pch.h"
#include "Input.h"

namespace Toad
{

	bool Input::IsKeyDown(sf::Keyboard::Key key)
	{
		return sf::Keyboard::isKeyPressed(key);
	}

	bool Input::IsMouseDown(sf::Mouse::Button button)
	{
		return sf::Mouse::isButtonPressed(button);
	}

	void Input::AddKeyPressCallback(FKEY_CALLBACK callback)
	{
		m_keyPressCallbacks.emplace_back(callback);
	}

	void Input::AddMousePressCallback(FMOUSE_CALLBACK callback)
	{
		m_mousePressCallbacks.emplace_back(callback);
	}

	void Input::AddKeyReleaseCallback(FKEY_CALLBACK callback)
	{
		m_keyUpCallbacks.emplace_back(callback);
	}

	void Input::AddMouseReleaseCallback(FMOUSE_CALLBACK callback)
	{
		m_mouseUpCallbacks.emplace_back(callback);
	}

	void Input::InvokeKeyPressCallbacks(sf::Keyboard::Key key)
	{
		for (const auto& c : m_keyPressCallbacks)
		{
			c(key);
		}
	}

	void Input::InvokeMousePressCallbacks(sf::Mouse::Button button)
	{
		for (const auto& c : m_mousePressCallbacks)
		{
			c(button);
		}
	}

	void Input::InvokeKeyReleaseCallbacks(sf::Keyboard::Key key)
	{
		for (const auto& c : m_keyUpCallbacks)
		{
			c(key);
		}
	}

	void Input::InvokeMouseReleaseCallbacks(sf::Mouse::Button button)
	{
		for (const auto& c : m_mouseUpCallbacks)
		{
			c(button);
		}
	}

}