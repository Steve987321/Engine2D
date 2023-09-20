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

	void Circle::Render(sf::RenderWindow& window)
	{
		// draw sfml circleshape object
		window.draw(m_circle);
	}

	void Circle::Render(sf::RenderTexture& texture)
	{
		// draw sfml circleshape object
		texture.draw(m_circle);
	}

	void Circle::Update()
	{
		// update attached scripts
		for (auto script : m_attached_scripts | std::views::values)
		{
			script->OnUpdate(this);
		}
	}
}
