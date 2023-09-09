#pragma once

#include "Object.h"

namespace Toad
{

class ENGINE_API Sprite : public Object
{
public:
	friend struct SpriteSystem;

	explicit Sprite(std::string_view name);
	~Sprite() override;

	sf::Sprite& GetSprite();

private:
	void Update(sf::RenderWindow& window) override;

	sf::Sprite m_sprite;

	static inline std::set<Sprite*> s_spriteInstances = {};
};

}
