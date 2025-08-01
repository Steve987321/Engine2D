#include "pch.h"
#include "Circle.h"

#include "engine/Engine.h"

namespace Toad
{
	Circle::Circle(std::string_view obj_name)
		//: position(m_circle)
	{
		name = obj_name;

		m_circle.setRadius(10);
        last_scale = m_circle.getScale();
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

	void Circle::SetTextureRect(const IntRect& rect)
	{
		m_circle.setTextureRect(rect);
	}

	const Vec2f& Circle::GetPosition() const
	{
        return m_objectPos;
	}

	void Circle::SetPosition(const Vec2f& position)
	{
		Object::SetPosition(position);

		m_circle.setPosition(position);
	}

    FloatRect Circle::GetBounds() const
    {
		return m_circle.getGlobalBounds();
    }

	float Circle::GetRadius() const
	{
		return m_circle.getRadius();
	}

	void Circle::SetRadius(float radius)
	{
		m_circle.setRadius(radius);
	}

	void Circle::SetScale(const Vec2f& scale)
    {
        last_scale = scale;
		m_circle.setScale(scale);
	}

	const Vec2f& Circle::GetScale() const
    {
        return last_scale;
	}

	void Circle::SetRotation(float degrees)
	{
		Object::SetRotation(degrees);
		m_circle.setRotation(sf::degrees(degrees));
	}

	float Circle::GetRotation()
	{
		return m_circle.getRotation().asDegrees();
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
			rect["left"] = tex_rect.position.x;
			rect["top"] = tex_rect.position.y;
			rect["width"] = tex_rect.size.x;
			rect["height"] = tex_rect.size.y;

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
