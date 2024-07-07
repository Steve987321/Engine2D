#pragma once

#include "engine/default_objects/Object.h"

#include "Text.h"

namespace Toad
{

struct ButtonStyle
{
	int border_width = 1;

	sf::Color border_color;
	sf::Color col;

	TextStyle text_style;
};

using BUTTON_ONHOVER_CALLBACK = std::function<void()>;
using BUTTON_ONHOVEREXIT_CALLBACK = std::function<void()>;
using BUTTON_ONPRESS_CALLBACK = std::function<void(sf::Keyboard::Key)>;

// button logic class with simple style 
class Button : public Object
{
public:
	explicit Button(std::string_view obj_name);

public:
	void AddHoverEnterCallback(const BUTTON_ONHOVER_CALLBACK& callback);
	void AddHoverExitCallback(const BUTTON_ONHOVEREXIT_CALLBACK& callback);
	void AddOnPressCallback(const BUTTON_ONPRESS_CALLBACK& callback);

public:
	void OnCreate() override;
	void OnDestroy() override;
	void Start() override;
	void Render(sf::RenderTarget& target) override;
	void Update() override;
	void FixedUpdate() override;
	void LateUpdate() override;

	const Vec2f& GetPosition() const override;
	void SetPosition(const Vec2f& position) override;

	float GetRotation() override;
	void SetRotation(float degrees) override;

	json Serialize() override;

private:
	std::vector<BUTTON_ONHOVER_CALLBACK> m_onHoverCallbacks{};
	std::vector<BUTTON_ONHOVEREXIT_CALLBACK> m_onHoverExitCallbacks{};
	std::vector<BUTTON_ONPRESS_CALLBACK> m_onPressCallbacks{};

	sf::Sprite m_sprite;
};

}