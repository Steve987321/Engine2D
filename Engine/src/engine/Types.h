#pragma once

using Vec2i = sf::Vector2i;
using Vec2f = sf::Vector2f;
using Vec2u = sf::Vector2u;
using Vec3i = sf::Vector3i;
using Vec3f = sf::Vector3f;
using Keyboard = sf::Keyboard;
using IntRect = sf::IntRect;
using Texture = sf::Texture;
using Color = sf::Color;

class ENGINE_API FloatRect : public sf::FloatRect
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
	void Expand(float v);
	bool Intersects(const FloatRect& other);
	bool Contains(const Vec2f& point);

private:
	using sf::FloatRect::intersects;

};

class ENGINE_API Mouse : public sf::Mouse
{
public: 
	static void SetVisible(bool visible);
	static const Vec2i& GetPosition();
	static void SetPosition(const Vec2i& pos);

private:
	using sf::Mouse::getPosition;
	using sf::Mouse::setPosition;
};

struct ENGINE_API AudioSource
{
	std::filesystem::path full_path;
	std::filesystem::path relative_path;
	sf::SoundBuffer sound_buffer;
	bool has_valid_buffer;
};

