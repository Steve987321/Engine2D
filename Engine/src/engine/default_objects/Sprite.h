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
	const Vec2f& GetPosition() const override;
	void SetPosition(const Vec2f& position) override;

	FloatRect GetBounds() const override;

	void SetRotation(float degrees) override;
	float GetRotation() override;

	const Vec2f& GetScale() const override;
	void SetScale(const Vec2f& scale) override;

	std::filesystem::path& GetTextureSourcePath();
	void SetTexture(const std::filesystem::path& relative_path, const sf::Texture* texture);
	json Serialize() override;

private:
	void Render(sf::RenderTarget& target) override;
	void Start() override;
	void Update() override;

private:
	sf::Sprite m_sprite;
	std::filesystem::path m_texture_source_path;
};

}
