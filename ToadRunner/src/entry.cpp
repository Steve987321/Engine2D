#include "pch.h"
#include <EngineCore.h>

#include <engine/Engine.h>
#include <Game/src/game_core/GameBase.h>

#include "ui/ui.h"

int main(int argc, char** argv)
{
	Toad::Engine engine;

#ifdef TOAD_EDITOR
	engine.SetEngineUI(ui::engine_ui);
#endif

	if (!engine.Init())
		return EXIT_FAILURE;

	engine.Run();

	return EXIT_SUCCESS;
}