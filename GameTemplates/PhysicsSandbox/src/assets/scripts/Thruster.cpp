#include "framework/Framework.h"
#include "Thruster.h"

#include "engine/Engine.h"

using namespace Toad;

// Called on scene begin 
void Thruster::OnStart(Object* obj)
{
	Script::OnStart(obj);

}

// Called every frame
void Thruster::OnUpdate(Object* obj)
{
	Script::OnUpdate(obj);

}

// Show variables in the editor and serialize to scene
void Thruster::ExposeVars()
{
	Script::ExposeVars();
	EXPOSE_VAR(thrust);
	EXPOSE_VAR(angle);
	EXPOSE_VAR(mass);
	EXPOSE_VAR(attached_obj_name);
}
