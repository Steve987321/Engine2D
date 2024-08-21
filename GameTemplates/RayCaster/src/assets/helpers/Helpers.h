#pragma once

enum class DIRECTION
{
	NONE,
	N,
	E,
	S,
	W,
};
struct HitSide
{
	Vec2f position;
	DIRECTION side;
};

bool LineIntersection(Vec2f p1, Vec2f p2, Vec2f p3, Vec2f p4, Vec2f& intersection);
bool LineRectIntersection(Vec2f p1, Vec2f p2, sf::FloatRect rect, std::vector<sf::Vector2f>& intersections);
bool LineRectIntersection(Vec2f p1, Vec2f p2, sf::FloatRect rect, std::vector<HitSide>& intersections);

float DegToRad(float deg);
