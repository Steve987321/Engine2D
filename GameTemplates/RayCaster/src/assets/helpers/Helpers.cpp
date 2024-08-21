#include "pch.h"
#include "engine/Engine.h"
#include "helpers.h"

bool LineIntersection(Vec2f p1, Vec2f p2, Vec2f p3, Vec2f p4, Vec2f& intersection)
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

bool LineRectIntersection(Vec2f p1, Vec2f p2, sf::FloatRect rect, std::vector<sf::Vector2f>& intersections)
{
	Vec2f intersection;
	bool hit = false;

	Vec2f top_left = Vec2f(rect.left, rect.top);
	Vec2f top_right = Vec2f(rect.left + rect.width, rect.top);
	Vec2f bottom_left = Vec2f(rect.left, rect.top + rect.height);
	Vec2f bottom_right = Vec2f(rect.left + rect.width, rect.top + rect.height);

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

bool LineRectIntersection(Vec2f p1, Vec2f p2, sf::FloatRect rect, std::vector<HitSide>& intersections)
{
	Vec2f intersection;
	bool hit = false;

	Vec2f top_left = Vec2f(rect.left, rect.top);
	Vec2f top_right = Vec2f(rect.left + rect.width, rect.top);
	Vec2f bottom_left = Vec2f(rect.left, rect.top + rect.height);
	Vec2f bottom_right = Vec2f(rect.left + rect.width, rect.top + rect.height);

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
