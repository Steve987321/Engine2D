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
    return std::sqrt(LengthSquared());
}

float Vec2f::LengthSquared() const
{
    return x * x + y * y;
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
    position.x -= v;
    position.y -= v;
    size.y += v;
    size.y += v;
}

bool FloatRect::Intersects(const FloatRect& other)
{
    Vec2f other_max = other.position + other.size;
    Vec2f max = position + size;
	if (position.x > other_max.x)
        return false;
    if (position.y > other_max.y)
        return false; 
    if (max.x > other.position.x)
        return false;
    if (max.y < other.position.y)
        return false; 

    return true; 
}

bool FloatRect::Contains(const Vec2f& point)
{
	return contains(point);
}

namespace Mouse
{

static bool mouse_visible = true;
static bool capture_mouse = false;
static Vec2i last_captured_mouse_pos {};
static Vec2i relative_mouse_pos {};

void SetVisible(bool visible)
{
#ifdef TOAD_EDITOR
	mouse_visible = visible;
	if (capture_mouse)
		GetWindow().setMouseCursorVisible(visible);
#else
	GetWindow().setMouseCursorVisible(visible);
#endif 
}

Vec2i GetPosition()
{
#ifdef TOAD_EDITOR
	if (!capture_mouse)
		return last_captured_mouse_pos;

	last_captured_mouse_pos = relative_mouse_pos;
	return last_captured_mouse_pos;
#endif 

	return GetPositionRaw();
}

Vec2i GetPositionRaw()
{
    return sf::Mouse::getPosition();
}

void SetPosition(const Vec2i& pos)
{
#ifdef TOAD_EDITOR
	if (!capture_mouse)
		return;
#endif 
	sf::Mouse::setPosition(pos);
}

void SetRelativeMousePosition(const Vec2i& pos)
{
    relative_mouse_pos = pos;
}

const Vec2i& GetRelativeMousePosition()
{
    return relative_mouse_pos;
}

void ShouldCaptureMouse(bool capture)
{
    capture_mouse = capture;
}

}

}
