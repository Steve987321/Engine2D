#pragma once

#include "engine/systems/Animation.h"
#include "engine/default_objects/Camera.h"

namespace Toad
{

namespace fs = std::filesystem;

// Used as an overlay for the viewport to edit/save/load animations
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

	sf::RenderTexture m_previewTexture;

	Toad::Camera m_cam{"AnimEditorCam"};

	// for inspector 
	std::function<void()> m_inspectorUI;

	std::vector<sf::Sprite> m_sequence{};
	sf::Sprite m_spriteSheet;

};

}