#include "pch.h"
#include "EngineCore.h"
#include "InputMac.h"

#include "engine/Logger.h"

namespace InputMac
{

using namespace sf; 

static std::array<bool, Keyboard::KeyCount> key_pressed_states;

void MacKeyPressCallback(Keyboard::Key key)
{
    key_pressed_states[(int)key] = true; 
}

void MacKeyReleaseCallback(Keyboard::Key key)
{
    key_pressed_states[(int)key] = false; 
}

bool IsKeyDown(Keyboard::Key key)
{
    return key_pressed_states[(int)key];
}

}
