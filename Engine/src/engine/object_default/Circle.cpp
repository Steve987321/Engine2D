#include "pch.h"
#include "Circle.h"

namespace Toad
{
	Circle::Circle(std::string_view obj_name)
	{
		name = obj_name;
	}

	Circle::~Circle() = default;

	sf::CircleShape& Circle::GetCircle()
	{
		return m_circle;
	}

	void Circle::Update(sf::RenderWindow& window)
	{
		// update attached scripts
		for (auto& script : m_attached_scripts)
		{
			script.OnUpdate(this);
		}

		// draw sfml circleshape object
		window.draw(m_circle);
	}

	void Circle::Update(sf::RenderTexture& texture)
	{
		// update attached scripts
		for (auto& script : m_attached_scripts)
		{
			script.OnUpdate(this);
		}

		// draw sfml circleshape object
		texture.draw(m_circle);
	}
}
