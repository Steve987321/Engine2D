#include "pch.h"
#include "Camera.h"

#include "engine/Engine.h"

namespace Toad
{

Camera::Camera(std::string_view obj_name)
{
	name = obj_name;
}

Camera& Camera::GetActiveCamera()
{
	
}

void Camera::Start()
{
	Object::Start();
}

void Camera::Render(sf::RenderWindow& window)
{
	Object::Render(window);
}

void Camera::Render(sf::RenderTexture& texture)
{
	Object::Render(texture);
}

void Camera::Update()
{
	Object::Update();
}

const Vec2f& Camera::GetPosition()
{
	return m_view.getCenter();
}

void Camera::SetPosition(const Vec2f& position)
{
	Object::SetPosition(position);
}

float Camera::GetRotation()
{
	return m_view.getRotation();
}

void Camera::SetRotation(float degrees)
{
	Object::SetRotation(degrees);

	m_view.setRotation(degrees);
}

void Camera::ActivateCamera()
{
	Engine::Get().GetWindow().setView(m_view);
}

void Camera::DeactivateCamera()
{
	Engine::Get().GetWindow().setView(m_view);
}

}