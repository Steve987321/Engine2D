#include "Game.h"

static AppSettings game_settings
{
	"Sim",
	1280,
	720,
	60,
	sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize,
};

AppSettings get_game_settings()
{
	return game_settings;
}
