#include "pch.h"
#include "framework/Framework.h"
#include "Player.h"

#include "Map.h"
#include "MapObject.h"
#include "RayCaster.h"
#include "helpers/helpers.h"

#include "engine/Types.h"

using namespace Toad;

static RayCaster* ray_caster = nullptr;
static Map* map = nullptr;

// Called on scene begin 
void Player::OnStart(Object* obj)
{
	Script::OnStart(obj);
	ray_caster = obj->GetScript<RayCaster>("RayCaster");
	if (!ray_caster)
		LOGERROR("[Player] Can't get RayCaster script in this object");

	auto map_obj = Scene::current_scene.GetSceneObject("GridController");
	if (map_obj)
		map = map_obj->GetScript<Map>("Map");
	else
		LOGERRORF("[Player] Can't find grid controller object");

	Mouse::SetVisible(false);
}

// Called every frame
void Player::OnUpdate(Object* obj)
{
	Script::OnUpdate(obj);
	
	float rotation_deg = obj->GetRotation();
	float rotation_rad = DegToRad(rotation_deg);
	float rotation_rad_right = DegToRad(rotation_deg + 90);
	Vec2f direction = {
		cosf(rotation_rad),
		sinf(rotation_rad),
	};
	
	Vec2f direction_right = {
		cosf(rotation_rad_right),
		sinf(rotation_rad_right),
	};

	vel = {0, 0};

	if (Input::IsKeyDown(Keyboard::W))
		vel += direction;

	if (Input::IsKeyDown(Keyboard::A))
		vel -= direction_right;

	if (Input::IsKeyDown(Keyboard::S))
		vel -= direction;

	if (Input::IsKeyDown(Keyboard::D))
		vel += direction_right;

	// mouse look 
	Vec2i mouse_pos = Mouse::GetPosition();

	// get current window 
	const sf::RenderWindow& window = Toad::GetWindow();

	// locks mouse at center of window (#TODO: including the editor)
	const Vec2i locked_mouse_pos = { 
		window.getPosition().x + (int)window.getSize().x / 2,  
		window.getPosition().y + (int)window.getSize().y / 2 };

	static Vec2i mouse_pos_prev = locked_mouse_pos;
	Vec2i mouse_pos_delta = mouse_pos - mouse_pos_prev;

	if (mouse_pos_delta.x != 0 || mouse_pos_delta.y != 0)
	{
		float dt = Time::GetDeltaTime();

		if (ray_caster)
			ray_caster->pitch = std::clamp(ray_caster->pitch - mouse_pos_delta.y * mouse_look_sens * dt, -70.f, 70.f);

		obj->SetRotation(rotation_deg + mouse_pos_delta.x * mouse_look_sens * dt);
	}

	// lock mouse 
	Mouse::SetPosition(locked_mouse_pos);
	mouse_pos_prev = locked_mouse_pos;
}

void Player::OnFixedUpdate(Toad::Object* obj)
{
	if (!map)
		return;

	Vec2f pos_prev = obj->GetPosition();
	Vec2f pos = pos_prev + vel * speed * Time::fixed_delta_time;
	FloatRect player_rect;
	player_rect.left = pos.x - player_size;
	player_rect.top = pos.y - player_size;
	player_rect.width = player_size * 2;
	player_rect.height = player_size * 2;

	Vec2f solved_pos = pos;

	for (const MapObject* object : map->objects)
	{
		const FloatRect& tile_position = object->spr->GetSprite().getGlobalBounds();

		if (player_rect.Intersects(tile_position))
		{
			float overlap_x = 0;
			if (vel.x > 0)
				overlap_x = (player_rect.left + player_rect.width) - tile_position.left;
			else if (vel.x < 0)
				overlap_x = tile_position.left + tile_position.width - player_rect.left;

			float overlap_y = 0;
			if (vel.y > 0)
				overlap_y = (player_rect.top + player_rect.height) - tile_position.top;
			else if (vel.y < 0)
				overlap_y = tile_position.top + tile_position.height - player_rect.top;

			if (std::abs(overlap_x) < std::abs(overlap_y))
			{
				if (vel.x > 0)
					solved_pos.x -= overlap_x;
				else if (vel.x < 0)
					solved_pos.x += overlap_x;
			}
			else
			{
				if (vel.y > 0)
					solved_pos.y -= overlap_y;
				else if (vel.y < 0)
					solved_pos.y += overlap_y;
			}
		}
	}

	obj->SetPosition(solved_pos);
}

// Show variables in the editor and serialize to scene
void Player::ExposeVars()
{
	Script::ExposeVars();

	EXPOSE_VAR(speed);
	EXPOSE_VAR(mouse_look_sens);
	EXPOSE_VAR(player_size);
}
