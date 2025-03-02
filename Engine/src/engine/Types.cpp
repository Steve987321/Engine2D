#include "pch.h"
#include "engine/Engine.h"
#include "Types.h"

namespace Toad
{

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

const Vec2i& Mouse::GetPosition()
{
#ifdef TOAD_EDITOR
	if (!capture_mouse)
		return last_captured_mouse_pos;

	last_captured_mouse_pos = getPosition();
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
