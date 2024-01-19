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

		std::filesystem::path& GetTextureSourcePath();
		void SetTexture(const std::filesystem::path& relative_path, const sf::Texture* texture);

		const Vec2f& GetPosition() override;
		void SetPosition(const Vec2f& position) override;

		void SetRotation(float degrees) override;
		float GetRotation() override;

		json Serialize();

	private:
		void Start() override;
		void Render(sf::RenderWindow& window) override;
		void Render(sf::RenderTexture& texture) override;
		void Update() override;

	private:
		sf::CircleShape m_circle;
		std::filesystem::path m_texture_source_path;
	};
}


