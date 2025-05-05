#include "pch.h"
#include "Button.h"

#include "engine/Engine.h"

namespace Toad
{

Button::Button(std::string_view obj_name)
	: m_sprite(*ResourceManager::GetTextures().Get("Default"))
{
	name = obj_name;
}

void Button::AddHoverEnterCallback(const BUTTON_ONHOVER_CALLBACK& callback)
{
	m_onHoverCallbacks.emplace_back(callback);
}

void Button::AddOnPressCallback(const BUTTON_ONPRESS_CALLBACK& callback)
{
	m_onPressCallbacks.emplace_back(callback);
}

void Button::AddHoverExitCallback(const BUTTON_ONHOVEREXIT_CALLBACK& callback)
{
	m_onHoverExitCallbacks.emplace_back(callback);
}

void Button::OnCreate()
{
	Object::OnCreate();
}

void Button::OnDestroy()
{
	Object::OnDestroy();
}

void Button::Start()
{
	Object::Start();
}

void Button::Render(sf::RenderTarget& target)
{
	target.draw(m_sprite);
}

void Button::Update()
{
	Object::Update();
}

void Button::FixedUpdate()
{
	Object::FixedUpdate();
}

void Button::LateUpdate()
{
	Object::LateUpdate();
}

const Vec2f& Button::GetPosition() const
{
	return m_objectPos;
}

void Button::SetPosition(const Vec2f& position)
{
	Object::SetPosition(position);

	m_sprite.setPosition(position);
}

float Button::GetRotation()
{
	return m_sprite.getRotation().asDegrees();
}

void Button::SetRotation(float degrees)
{
	m_sprite.setRotation(sf::degrees(degrees));
	Object::SetRotation(degrees);
}

json Button::Serialize()
{
	return SerializeScripts();
}

}