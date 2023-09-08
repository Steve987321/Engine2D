#include "pch.h"
#include "IComponent.h"

namespace Toad
{

void Component::CallComponentsStart()
{
	for (auto& component : m_components)
	{
		component.Start();
	}
}

void Component::CallComponentsUpdate()
{
	for (auto& component : m_components)
	{
		component.Update();
	}
}

}

