#include "pch.h"
#include "Camera.h"

#include "engine/Engine.h"

namespace Toad
{

Camera::Camera(std::string_view obj_name)
{
	name = obj_name;
}

json Camera::Serialize()
{
	throw std::logic_error("The method or operation is not implemented.");
}

std::vector<Camera*> Camera::GetActiveCameras()
{
	std::vector<Camera*> active_cams;

	for (Camera* cam : m_cameras)
	{
		if (cam->IsActive())
		{
			active_cams.push_back(cam);
		}
	}

	return active_cams;
	
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

bool Camera::IsActive() const
{
	return m_active;
}

void Camera::ActivateCamera()
{
	m_active = true;
	Engine::Get().GetWindow().setView(m_view);
}

void Camera::DeactivateCamera()
{
	m_active = false;
	Engine::Get().GetWindow().setView(m_view);
}

void Camera::SetSize(const Vec2f& size)
{
}

const Vec2f& Camera::GetSize() const
{
	return m_view.getSize();
}

void Camera::Zoom(float factor)
{
}

void Camera::SetViewport(const sf::FloatRect& viewport)
{
}
}
