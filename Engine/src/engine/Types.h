#pragma once

using Vec2i = sf::Vector2i;
using Vec2f = sf::Vector2f;
using Vec2u = sf::Vector2u;
using Vec3i = sf::Vector3i;
using Vec3f = sf::Vector3f;
using Mouse = sf::Mouse;
using Keyboard = sf::Keyboard;
using IntRect = sf::IntRect;

class FloatRect : public sf::FloatRect
{
public:
	FloatRect()
		: sf::FloatRect()
	{}

	FloatRect(const sf::FloatRect& r)
	{
		width = r.width;
		height = r.height;
		left = r.left;
		top = r.top;
	}

public:
	void Expand(float v)
	{
		width += v;
		height += v;
		left -= v;
		top -= v;
	}

	bool Intersects(const FloatRect& other)
	{
		return intersects(other);
	}

	bool Contains(const Vec2f& point)
	{
		return point.x > left && point.x < left + width && point.y > top && point.y < top + height;
	}

private:
	using sf::FloatRect::intersects;

};

struct AudioSource
{
	std::filesystem::path full_path;
	std::filesystem::path relative_path;
	sf::SoundBuffer sound_buffer;
	bool has_valid_buffer;
};
