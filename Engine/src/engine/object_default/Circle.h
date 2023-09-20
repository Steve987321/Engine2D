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
		void Start() override;
		void Render(sf::RenderWindow& window) override;
		void Render(sf::RenderTexture& texture) override;
		void Update() override;

		sf::CircleShape m_circle;
	};
}


