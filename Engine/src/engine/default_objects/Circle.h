#pragma once

#include "Object.h"

namespace Toad
{

	struct CircleColorProxy
	{
		
	};

	class ENGINE_API Circle : public Object
	{
	public:
		explicit Circle(std::string_view obj_name);
		~Circle() override;

	public:
		sf::CircleShape& GetCircle();

		std::filesystem::path& GetTextureSourcePath();
		void SetTexture(const std::filesystem::path& relative_path, const sf::Texture* texture);
		void SetTextureRect(const IntRect& rect);

		const Vec2f& GetPosition() const override;
		void SetPosition(const Vec2f& position) override;

		void SetScale(const Vec2f& scale) override;
		const Vec2f& GetScale() const override;

		FloatRect GetBounds() const override;

		float GetRadius() const;
		void SetRadius(float radius);

		sf::Color GetFillColor() const;
		void SetFillColor(const sf::Color& col);
		sf::Color GetOutlineColor() const;
		void SetOutlineColor(const sf::Color& col);

		void SetRotation(float degrees) override;
		float GetRotation() override;

		json Serialize() override;

	private:
		void Start() override;
		void Render(sf::RenderTarget& target) override;
		void Update() override;

	private:
		sf::CircleShape m_circle;
		std::filesystem::path m_texture_source_path;
	};
}


