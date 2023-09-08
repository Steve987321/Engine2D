#pragma once

#include "Object.h"

namespace Toad
{

class Sprite : public Object
{
public:
	friend struct SpriteSystem;

	Sprite();
	~Sprite() override;

	sf::Sprite& GetSprite();

private:
	void Update(sf::RenderWindow& window) override;

	sf::Sprite m_sprite;

	static inline std::set<Sprite*> s_spriteInstances = {};
};

}
