#include "pch.h"
#include "SpriteSystem.h"

namespace Toad
{

void SpriteSystem::UpdateSpriteObjects(sf::RenderWindow& window)
{
	for (auto s : Sprite::s_spriteInstances)
	{
		s->Update(window);
	}
}

}