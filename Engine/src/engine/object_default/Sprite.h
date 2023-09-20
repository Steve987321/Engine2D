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
	void Render(sf::RenderWindow& window) override;
	void Render(sf::RenderTexture& texture) override;
	void Update() override;

	sf::Sprite m_sprite;
};

}
