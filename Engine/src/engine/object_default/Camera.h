#pragma once

#include "Object.h"

namespace Toad
{

class Camera : public Object
{
public:
	explicit Camera(std::string_view obj_name);

public:
	static Camera& GetActiveCamera();

public:

	void Start() override;
	void Render(sf::RenderWindow& window) override;
	void Render(sf::RenderTexture& texture) override;
	void Update() override;
	const Vec2f& GetPosition() override;
	void SetPosition(const Vec2f& position) override;

	float GetRotation() override;
	void SetRotation(float degrees) override;

	void ActivateCamera();
	void DeactivateCamera();

	void SetSize(const Vec2f& size);
	const Vec2f& GetSize() const;

	void Zoom(float factor);

	void SetViewport(const sf::FloatRect& viewport);

private:
	sf::View m_view;
	static std::vector<Camera*> m_cameras;

};

}