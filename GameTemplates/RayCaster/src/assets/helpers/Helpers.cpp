#include "pch.h"
#include "engine/Engine.h"
#include "Helpers.h"

bool LineIntersection(Toad::Vec2f p1, Toad::Vec2f p2, Toad::Vec2f p3, Toad::Vec2f p4, Toad::Vec2f& intersection)
{
	float denom = (p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y);
	if (std::abs(denom) < FLT_EPSILON)
		return false;

	float ua = ((p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x)) / denom;
	float ub = ((p2.x - p1.x) * (p1.y - p3.y) - (p2.y - p1.y) * (p1.x - p3.x)) / denom;

	if (ua >= 0 && ua <= 1 && ub >= 0 && ub <= 1)
	{
		intersection.x = p1.x + ua * (p2.x - p1.x);
		intersection.y = p1.y + ua * (p2.y - p1.y);
		return true;
	}

	return false;
}

bool LineRectIntersection(Toad::Vec2f p1, Toad::Vec2f p2, sf::FloatRect rect, std::vector<sf::Vector2f>& intersections)
{
	Toad::Vec2f intersection;
	bool hit = false;

	Toad::Vec2f top_left = rect.position;
	Toad::Vec2f top_right { rect.position.x + rect.size.x, rect.position.y };
	Toad::Vec2f bottom_left  { rect.position.x, rect.position.y + rect.size.y };
	Toad::Vec2f bottom_right = rect.position + rect.size;

	if (LineIntersection(p1, p2, top_left, top_right, intersection))
	{
		intersections.emplace_back(intersection);
		hit = true;
	}
	if (LineIntersection(p1, p2, top_right, bottom_right, intersection))
	{
		intersections.emplace_back(intersection);
		hit = true;
	}
	if (LineIntersection(p1, p2, bottom_right, bottom_left, intersection))
	{
		intersections.emplace_back(intersection);
		hit = true;
	}
	if (LineIntersection(p1, p2, bottom_left, top_left, intersection))
	{
		intersections.emplace_back(intersection);
		hit = true;
	}

	return hit;
}

bool LineRectIntersection(Toad::Vec2f p1, Toad::Vec2f p2, sf::FloatRect rect, std::vector<HitSide>& intersections)
{
	Toad::Vec2f intersection;
	bool hit = false;

	Toad::Vec2f top_left = rect.position;
	Toad::Vec2f top_right { rect.position.x + rect.size.x, rect.position.y };
	Toad::Vec2f bottom_left  { rect.position.x, rect.position.y + rect.size.y };
	Toad::Vec2f bottom_right = rect.position + rect.size;

	if (LineIntersection(p1, p2, top_left, top_right, intersection))
	{
		intersections.emplace_back(intersection, DIRECTION::N);
		hit = true;
	}
	if (LineIntersection(p1, p2, top_right, bottom_right, intersection))
	{
		intersections.emplace_back(intersection, DIRECTION::E);
		hit = true;
	}
	if (LineIntersection(p1, p2, bottom_right, bottom_left, intersection))
	{
		intersections.emplace_back(intersection, DIRECTION::S);
		hit = true;
	}
	if (LineIntersection(p1, p2, bottom_left, top_left, intersection))
	{
		intersections.emplace_back(intersection, DIRECTION::W);
		hit = true;
	}

	return hit;
}

float DegToRad(float deg)
{
	return deg * std::numbers::pi_v<float> / 180.f;
}
