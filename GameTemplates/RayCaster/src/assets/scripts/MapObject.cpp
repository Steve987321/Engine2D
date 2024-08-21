#include "pch.h"
#include "framework/Framework.h"
#include "MapObject.h"

#include "Engine/Engine.h"

using namespace Toad;

// Called on scene begin 
void MapObject::OnStart(Object* obj)
{
	Script::OnStart(obj);
	spr = get_object_as_type<Sprite>(obj);
	if (!spr)
	{
		LOGERRORF("[MapObject] Object isn't of type Sprite, {}", obj->name);
		active = false;
		return;
	}
}

// Called every frame
void MapObject::OnUpdate(Object* obj)
{
	Script::OnUpdate(obj);

}

// Show variables in the editor and serialize to scene
void MapObject::ExposeVars()
{
	Script::ExposeVars();
	EXPOSE_VAR(active);
}
