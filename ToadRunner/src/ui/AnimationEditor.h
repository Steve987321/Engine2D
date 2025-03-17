#pragma once

#include "engine/systems/Animation.h"
#include "engine/default_objects/Camera.h"

namespace Toad
{

namespace fs = std::filesystem;

struct SpriteAnimation
{
	sf::Sprite* spr;
	std::vector<Vec2u> coordinates;
};

// edit, load and save animations 
class AnimationEditor
{
public:
	AnimationEditor();
	~AnimationEditor();

public:
	void Show(bool* show);

private:
	// Load png sequence animation
	void LoadAnimationFromSequence();

	// Load tilesheet sequence 
	void LoadAnimationFromTileSheet(const fs::path& path);

	// Inspector ui
	void ShowAnimationPropsUI();
private: 
	// Animation which is being worked on
	Animation m_selectedAnimation;
	SpriteAnimation m_spriteAnimation;

	sf::RenderTexture m_previewTexture;
	sf::RenderTexture m_previewOutputAnimationTexture;

	Toad::Camera m_cam{"AnimEditorCam"};
	Toad::Camera m_camOutput{"AnimOutputCam"};

	// for inspector 
	std::function<void()> m_inspectorUI;

	std::vector<std::string> m_textureIds{};
	std::vector<sf::Sprite> m_sequence{};
	sf::Sprite m_spriteSheet;

};

}
