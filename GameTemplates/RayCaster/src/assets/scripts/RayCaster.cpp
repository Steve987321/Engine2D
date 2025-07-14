#include "pch.h"
#include "framework/Framework.h"
#include "RayCaster.h"
#include "MapObject.h"
#include "Map.h"

#include "../helpers/Helpers.h"

using namespace Toad;

static Map* map = nullptr;

// temp vertical lines for rendering the '3d' screen
static std::array<Sprite*, RayCaster::fov> sprites;

//static std::array<Sprite*, RayCaster::fov> debug_sprites;

// default texture properties for the line sprites
static const sf::IntRect default_rect{0, 0, 1, 1};
static const sf::Texture* default_texture;

// sprites that hold textures by index
//static std::unordered_map<int, bool> sprites_with_texture;

void RayCaster::OnStart(Object* obj)
{
	Script::OnStart(obj);

	// set origin of this object (for better visuals)
	Circle* circle = GetObjectAsType<Circle>(obj);
	if (circle) 
	{
		float half_radius = circle->GetCircle().getRadius() / 2.f;
		circle->GetCircle().setOrigin(half_radius, half_radius);
	}

	std::shared_ptr<Object> floor_obj = Scene::current_scene.GetSceneObject("Floor");
	if (floor_obj)
		floor_sprite = GetObjectAsType<Sprite>(floor_obj.get());

	if (!floor_obj || !floor_sprite)
		LOGERRORF("[RayCaster] Can't find floor object, obj:{}, sprite:{}", (void*)floor_obj.get(), (void*)floor_sprite);

	std::shared_ptr<Object> map_object = Scene::current_scene.GetSceneObject("GridController");
	if (map_object)
		map = map_object->GetScript<Map>("Map");

	if (!map_object || !map)
		LOGERRORF("[RayCaster] Can't find grid controller object script: {} obj:{}", map, map_object);

	//for (auto& sprite : debug_sprites)
	//{
	//	sprite = Scene::current_scene.AddToScene(Sprite("DEBUGSPRITE")).get();
	//	sprite->SetParent(obj);
	//	sprite->GetSprite().setColor(sf::Color(255, 0, 0, 255));
	//	sprite->GetSprite().setTextureRect(rect);
	//	sprite->GetSprite().setOrigin({0.5f, 0.5f});
	//}

	for (Sprite*& sprite : sprites)
	{
		sprite = Scene::current_scene.AddToScene(Sprite("sprotes")).get();
		sprite->GetSprite().setTextureRect(default_rect);
		sprite->GetSprite().setOrigin({ 0.5f, 0.5f });
	}

	default_texture = sprites[0]->GetSprite().getTexture();
}

// Called every frame
void RayCaster::OnUpdate(Object* obj)
{
	Script::OnUpdate(obj);

	if (!map)
		return;

	const int max_ray_dist = 100;

	const Toad::Vec2f& pos = obj->GetPosition();
	int direction_deg = (int)obj->GetRotation();

	// counter for checking when a new line can be drawn
	int iterator_counter_line_thickness = 0;

	int screenx_coord_iterator = 0;

	for (int i = direction_deg - fovhalf; i < direction_deg + fovhalf; i++)
	{
		// ray end using the max looking distance
		float ray_dir_rad = DegToRad(i);
		Toad::Vec2f ray_end = pos + Toad::Vec2f{ 
			cosf(ray_dir_rad) * max_ray_dist, 
			sinf(ray_dir_rad) * max_ray_dist };
		
		float ray_hit_distance = FLT_MAX;
		Toad::Vec2f hit = pos;
		DIRECTION hit_side = DIRECTION::NONE;
		std::vector<HitSide> ray_collisions{};
		uint32_t grid_hit = -1;

		std::unordered_map<uint32_t, uint32_t> hits_per_map_obj;

		// check collision with existing tile 
		for (uint32_t j = 0; j < map->objects.size(); j++)
		{
			if (!map->objects[j]->active)
				continue;

			if (LineRectIntersection(pos, ray_end, map->objects[j]->spr->GetSprite().getGlobalBounds(), ray_collisions))
			{
				if (grid_hit != -1 && grid_hit != j)
				{
					float dist_a = distance(map->objects[grid_hit]->spr->GetPosition(), obj->GetPosition());
					float dist_b = distance(map->objects[j]->spr->GetPosition(), obj->GetPosition());

					if (dist_a > dist_b)
						grid_hit = j;
				}
				else 
					grid_hit = j;
			}
		}

		for (const auto& [intersection, side] : ray_collisions)
		{
			float dist = distance(intersection, pos);
			if (dist < ray_hit_distance)
			{
				ray_hit_distance = dist;
				hit = intersection;
				hit_side = side;
			}
		}

		//debug_sprites[screenx_coord_iterator]->SetPosition(hit);

		Sprite* line_obj = sprites[screenx_coord_iterator];
		sf::Sprite& line_spr = line_obj->GetSprite();
		float ylevel = 35.f + pitch;
		line_obj->SetPosition({(float)screenx_coord_iterator * line_thickness + 10, ylevel });
		line_spr.setScale({ (float)line_thickness, (float)max_ray_dist / ray_hit_distance * 5 });

		if (grid_hit != -1)
		{
			sf::Sprite& hit_spr = map->objects[grid_hit]->spr->GetSprite();
			sf::Color col(hit_spr.getColor());
			line_spr.setColor(col);

			// #TODO: Texture handling 
			const sf::Texture* texture = hit_spr.getTexture();

			if (texture)
			{
				FloatRect hit_obj_bounds = hit_spr.getGlobalBounds();
				IntRect texture_rect = hit_spr.getTextureRect();
				texture_rect.width = 1;

				const auto calc_texcoordx = [&texture_rect](float a, float b, float c)
					{
						float t = (c - a) / (b - a);
						texture_rect.left = (int)((float)texture_rect.left + t * (texture_rect.left + texture_rect.height - texture_rect.left));
					};

				float t = 0.f;
				switch (hit_side)
				{
				case DIRECTION::N:
					calc_texcoordx(hit_obj_bounds.left + hit_obj_bounds.width, hit_obj_bounds.left, hit.x);
					break;
				case DIRECTION::E:
					calc_texcoordx(hit_obj_bounds.top + hit_obj_bounds.height, hit_obj_bounds.top, hit.y);
					break;
				case DIRECTION::S:
					calc_texcoordx(hit_obj_bounds.left, hit_obj_bounds.left + hit_obj_bounds.width, hit.x);
					break;
				case DIRECTION::W:
					calc_texcoordx(hit_obj_bounds.top, hit_obj_bounds.top + hit_obj_bounds.height, hit.y);
					break;
				case DIRECTION::NONE:
					break;
				default:
					break;
				}

				line_spr.setTexture(*texture);
				line_spr.setTextureRect(texture_rect);
				Vec2i size_newi = texture_rect.getSize();
				Toad::Vec2f size_new{ (float)size_newi.x, (float)size_newi.y };

				Toad::Vec2f new_scale;
				new_scale.x = 1;
				new_scale.y = line_spr.getScale().y / size_new.y;

				line_spr.setOrigin({0.5f, (float)texture_rect.height / 2});
				line_spr.setScale(new_scale);
			}
		}

		if (floor_sprite)
			floor_sprite->SetPosition({ 0, ylevel });

		screenx_coord_iterator++;
	}
} 

// Show variables in the editor
void RayCaster::ExposeVars()
{
	Script::ExposeVars();

	EXPOSE_VAR(pitch);
}
