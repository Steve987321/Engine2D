#pragma once 

#ifdef __APPLE__

#include "SFML/Window/Keyboard.hpp"

namespace InputMac
{
    void MacKeyPressCallback(sf::Keyboard::Key key);
    void MacKeyReleaseCallback(sf::Keyboard::Key key);
    bool IsKeyDown(sf::Keyboard::Key key);
}

#endif