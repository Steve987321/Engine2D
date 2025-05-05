#include "pch.h"
#include "Camera.h"

#include "engine/Engine.h"

namespace Toad
{

Camera::Camera(std::string_view obj_name)
{
	name = obj_name;
}

Camera::~Camera()
{
	std::vector<Camera*>::iterator it = m_cameras.begin();

	while (it != m_cameras.end())
	{
		if ((*it)->name == name)
		{
			it = m_cameras.erase(it);
			break;
		}

		it++;
	}
	
	if (m_activeCam != nullptr)
	{
		if (m_activeCam == this)
		{
			m_activeCam = nullptr;
		}
	}
}

json Camera::Serialize()
{
	json data;
	json properties;
	json attached_scripts = SerializeScripts();

	properties["parent"] = GetParent();

	properties["posx"] = GetPosition().x;
	properties["posy"] = GetPosition().y;
	properties["rotation"] = GetRotation();

	properties["cam_active"] = m_active;
	properties["sizex"] = GetSize().x;
	properties["sizey"] = GetSize().y;
	
	data["properties"] = properties;
	data["scripts"] = attached_scripts;

	return data;
}

Camera* Camera::GetActiveCamera()
{
#ifdef TOAD_EDITOR
	if (m_activeCam)
        return m_activeCam;
    else
        return GetInteractingCamera();
#else
    return m_activeCam;
#endif
}

void Camera::OnCreate()
{
	m_cameras.push_back(this);
    Vec2u size = GetWindow().getSize();

    SetPosition(m_view.getCenter());

	m_view.setSize({(float)size.x, (float)size.y});
    last_size = Vec2f{(float)size.x, (float)size.y};

    original_size = m_view.getSize();
}

void Camera::Start()
{
	Object::Start();
}

void Camera::Render(sf::RenderTarget& target)
{
	Object::Render(target);
}

void Camera::Update()
{
	Object::Update();
}

const Vec2f& Camera::GetPosition() const
{
    return m_objectPos;
}

void Camera::SetPosition(const Vec2f& position)
{
	Object::SetPosition(position);

	m_view.setCenter(position);
}

float Camera::GetRotation()
{
	return m_view.getRotation().asDegrees();
}

void Camera::SetRotation(float degrees)
{
	Object::SetRotation(degrees);

	m_view.setRotation(sf::degrees(degrees));
}

bool Camera::IsActive() const
{
	return m_active;
}

void Camera::ActivateCamera()
{
	m_active = true;
	if (m_activeCam != nullptr)
	{
		m_activeCam->DeactivateCamera();
	}
	m_activeCam = this;
}

void Camera::DeactivateCamera()
{
	m_active = false;

	if (m_activeCam != nullptr)
	{
		if (name == m_activeCam->name)
		{
			m_activeCam = nullptr;
		}
	}
}

void Camera::SetSize(const Vec2f& size)
{
    last_size = size;
	m_view.setSize(size);
}

const Vec2f& Camera::GetSize() const
{
    // return m_view.getSize crashes
    return last_size;
}

void Camera::Zoom(float factor)
{
	m_view.zoom(factor);
}

void Camera::SetViewport(const sf::FloatRect& viewport)
{
	m_view.setViewport(viewport);
}

const sf::View& Camera::GetView() const
{
	return m_view;
}

}
