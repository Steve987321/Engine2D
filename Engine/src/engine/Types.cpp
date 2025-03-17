#include "pch.h"
#include "engine/Engine.h"
#include "Types.h"

namespace Toad
{

Vec2f::Vec2f()
    : sf::Vector2f()
{
}

Vec2f::Vec2f(float x, float y)
    : sf::Vector2f(x, y)
{
}

Vec2f::Vec2f(const Vec2f& v)
    : sf::Vector2f(v)
{
}

Vec2f::Vec2f(const sf::Vector2f& v)
    : sf::Vector2f(v)
{
}

void Vec2f::operator=(const sf::Vector2f& other)
{
    x = other.x;
    y = other.y;
}

Vec2f Vec2f::operator+(const Vec2f& other) const
{
    return {x + other.x, y + other.y};
}

Vec2f Vec2f::operator-(const Vec2f& other) const
{
    return {x - other.x, y - other.y};
}

Vec2f Vec2f::operator*(const Vec2f& other) const
{
    return {x * other.x, y * other.y};
}

Vec2f Vec2f::operator/(const Vec2f& other) const
{
    return {x / other.x, y / other.y};
}

Vec2f Vec2f::operator+(float scalar) const
{
    return {x + scalar, y + scalar};
}

Vec2f Vec2f::operator*(float scalar) const
{
    return {x * scalar, y * scalar};
}

Vec2f Vec2f::operator/(float scalar) const
{
    return {x / scalar, y / scalar};
}

void Vec2f::operator+=(const Vec2f& other)
{
    x += other.x;
    y += other.y;
}

void Vec2f::operator-=(const Vec2f& other)
{
    x -= other.x;
    y -= other.y;
}

void Vec2f::operator-=(float scalar)
{
    x -= scalar;
    y -= scalar;
}

void Vec2f::operator+=(float scalar)
{
    x += scalar;
    y += scalar;
}

void Vec2f::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
}

void Vec2f::operator/=(float scalar)
{
    x /= scalar;
    y /= scalar;
}

float Vec2f::Length() const
{
    return std::sqrt(x * x + y * y);
}

Vec2f Vec2f::Normalize() const
{
    float length = Length();
    return {x / length, y / length};
}

float Vec2f::Cross(const Vec2f &v) const
{
    return x * v.y - y * v.x;
}

float Vec2f::Dot(const Vec2f& v) const
{
    return x * v.x + y * v.y;
}


void FloatRect::Expand(float v)
{
	width += v;
	height += v;
	left -= v;
	top -= v;
}

bool FloatRect::Intersects(const FloatRect& other)
{
	return intersects(other);
}

bool FloatRect::Contains(const Vec2f& point)
{
	return point.x > left && point.x < left + width && point.y > top && point.y < top + height;
}

void Mouse::SetVisible(bool visible)
{
#ifdef TOAD_EDITOR
	mouse_visible = visible;
	if (capture_mouse)
		GetWindow().setMouseCursorVisible(visible);
#else
	GetWindow().setMouseCursorVisible(visible);
#endif 
}

Vec2i Mouse::GetPosition()
{
#ifdef TOAD_EDITOR
	if (!capture_mouse)
		return last_captured_mouse_pos;

	last_captured_mouse_pos = relative_mouse_pos;
	return last_captured_mouse_pos;
#endif 

	return getPosition();
}

void Mouse::SetPosition(const Vec2i& pos)
{
#ifdef TOAD_EDITOR
	if (!capture_mouse)
		return;
#endif 
	setPosition(pos);
}

}
