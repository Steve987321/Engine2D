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

	json Circle::Serialize()
	{
		json c_data;
		json circle_properties;
		json attached_scripts;

		for (const auto& it : GetAttachedScripts())
		{
			const auto& reflection_vars = it.second->GetReflection().Get();
			const auto& bs = reflection_vars.b;
			const auto& flts = reflection_vars.flt;
			const auto& i8s = reflection_vars.i8;
			const auto& i16s = reflection_vars.i16;
			const auto& i32s = reflection_vars.i32;
			const auto& strs = reflection_vars.str;

			auto bs_data = json::object();
			for (const auto& [name, val] : bs)
			{
				bs_data[name] = *val;
			}
			auto flts_data = json::object();
			for (const auto& [name, val] : flts)
			{
				flts_data[name] = *val;
			}
			auto i8s_data = json::object();
			for (const auto& [name, val] : i8s)
			{
				i8s_data[name] = *val;
			}
			auto i16s_data = json::object();
			for (const auto& [name, val] : i16s)
			{
				i16s_data[name] = *val;
			}
			auto i32s_data = json::object();
			for (const auto& [name, val] : i32s)
			{
				i32s_data[name] = *val;
			}
			auto strs_data = json::object();
			for (const auto& [name, val] : strs)
			{
				strs_data[name] = *val;
			}
			attached_scripts[it.first] =
			{
				bs_data,
				flts_data,
				i8s_data,
				i16s_data,
				i32s_data,
				strs_data
			};
		}

		const auto& c = GetCircle();

		circle_properties["posx"] = c.getPosition().x;
		circle_properties["posy"] = c.getPosition().y;
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
		for (auto script : m_attached_scripts | std::views::values)
		{
			script->OnStart(this);
		}
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
		for (const auto& script : m_attached_scripts | std::views::values)
		{
			script->OnUpdate(this);
		}
	}
}
