#pragma once

#include "Object.h"

namespace Toad
{
	class ENGINE_API Circle : public Object
	{
	public:
		explicit Circle(std::string_view obj_name);
		~Circle() override;

		sf::CircleShape& GetCircle();

	private:
		void Update(sf::RenderWindow& window) override;
		void Update(sf::RenderTexture& texture) override;

		sf::CircleShape m_circle;
	};
}


