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
	Toad::Vec2f position;
	DIRECTION side;
};

bool LineIntersection(Toad::Vec2f p1, Toad::Vec2f p2, Toad::Vec2f p3, Toad::Vec2f p4, Toad::Vec2f& intersection);
bool LineRectIntersection(Toad::Vec2f p1, Toad::Vec2f p2, sf::FloatRect rect, std::vector<sf::Vector2f>& intersections);
bool LineRectIntersection(Toad::Vec2f p1, Toad::Vec2f p2, sf::FloatRect rect, std::vector<HitSide>& intersections);

float DegToRad(float deg);
