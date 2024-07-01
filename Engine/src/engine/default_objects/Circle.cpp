#include "pch.h"
#include "Circle.h"

namespace Toad
{
	Circle::Circle(std::string_view obj_name)
	{
		name = obj_name;

		m_circle.setRadius(10);
	}

	Circle::~Circle() = default;

	sf::CircleShape& Circle::GetCircle()
	{
		return m_circle;
	}

	std::filesystem::path& Circle::GetTextureSourcePath()
	{
		return m_texture_source_path;
	}

	void Circle::SetTexture(const std::filesystem::path& relative_path, const sf::Texture* texture)
	{
		m_texture_source_path = relative_path;
		m_circle.setTexture(texture, true);
	}

	const Vec2f& Circle::GetPosition() const 
	{
		return m_circle.getPosition();
	}

	void Circle::SetPosition(const Vec2f& position)
	{
		Object::SetPosition(position);

		m_circle.setPosition(position);
	}

	void Circle::SetRotation(float degrees)
	{
		Object::SetRotation(degrees);

		m_circle.setRotation(degrees);
	}

	float Circle::GetRotation()
	{
		return m_circle.getRotation();
	}

	json Circle::Serialize()
	{
		json c_data;
		json circle_properties;
		json attached_scripts = SerializeScripts();

		const auto& c = GetCircle();

		circle_properties["parent"] = GetParent();

		circle_properties["posx"] = GetPosition().x;
		circle_properties["posy"] = GetPosition().y;
		circle_properties["rotation"] = GetRotation();
		circle_properties["fill_col"] = c.getFillColor().toInteger();
		circle_properties["outline_col"] = c.getOutlineColor().toInteger();
		circle_properties["radius"] = c.getRadius();
		circle_properties["scalex"] = c.getScale().x;
		circle_properties["scaley"] = c.getScale().y;
		bool has_texture = c.getTexture() != nullptr;
		if (has_texture)
		{
			circle_properties["texture_loc"] = GetTextureSourcePath();
			sf::IntRect tex_rect = c.getTextureRect();
			json rect;
			rect["left"] = tex_rect.left;
			rect["top"] = tex_rect.top;
			rect["width"] = tex_rect.width;
			rect["heigth"] = tex_rect.height;

			circle_properties["texture_rect"] = rect;
		}
		circle_properties["has_texture"] = has_texture;

		c_data["properties"] = circle_properties;
		c_data["scripts"] = attached_scripts;

		return c_data;
	}

	void Circle::Start()
	{
		Object::Start();
	}

	void Circle::Render(sf::RenderTarget& target)
	{
		target.draw(m_circle);
	}

	void Circle::Update()
	{
		Object::Update();
	}
}
