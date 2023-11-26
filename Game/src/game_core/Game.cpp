#include "pch.h"
#include "Game.h"

std::vector<Toad::Scene> get_scenes()
{
	return game_scenes;
}

void set_scenes(const std::vector<Toad::Scene>& scenes)
{
	game_scenes = scenes;
}

AppSettings get_game_settings()
{
	return game_settings;
}
