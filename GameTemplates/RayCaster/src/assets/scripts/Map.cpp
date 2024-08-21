#include "pch.h"
#include "framework/Framework.h"
#include "Map.h"

#include "MapObject.h"

#include "Engine/Engine.h"

using namespace Toad;

Engine& engine = Engine::Get();

// Called on scene begin 
void Map::OnStart(Object* obj)
{
	Script::OnStart(obj);

	for (auto& object : Scene::current_scene.objects_all)
	{
		MapObject* map_obj_script = object->GetScript<MapObject>("MapObject");
		if (map_obj_script)
			objects.emplace_back(map_obj_script);
	}

}

// Called every frame
void Map::OnUpdate(Object* obj)
{
	Script::OnUpdate(obj);

}
//
//#if defined(TOAD_EDITOR) || !defined(NDEBUG)
//void Map::OnImGui(Object* obj, ImGuiContext* ctx)
//{
//	ImGui::SetCurrentContext(ctx);
//
//	// small simple grid editor 
//	ImGui::Begin("[Grid] Grid&MapObject Edit");
//	{
//		for (uint32_t i = 0; i < grid_sizex; i++)
//		{
//			for (uint32_t j = 0; j < grid_sizey; j++)
//			{
//				
//			}
//		}
//	}
//	ImGui::End();
//}
//#endif

// Show variables in the editor
void Map::ExposeVars()
{
	Script::ExposeVars();

}
