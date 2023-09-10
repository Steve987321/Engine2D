#pragma once

#include "Object.h"

namespace Toad
{

class ENGINE_API Sprite : public Object
{
public:
	explicit Sprite(std::string_view obj_name);
	~Sprite() override;

	sf::Sprite& GetSprite();

private:
	void Update(sf::RenderWindow& window) override;
	void Update(sf::RenderTexture& texture) override;

	sf::Sprite m_sprite;
};

}
