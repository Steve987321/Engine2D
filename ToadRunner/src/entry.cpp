#include "pch.h"
#include <EngineCore.h>

#include <engine/Engine.h>
#include "../Game/src/game_core/ScriptRegister.h"

#include "ui/ui.h"

int main(int argc, char** argv)
{
	Toad::Engine engine;

#ifdef TOAD_EDITOR
	engine.SetEngineUI(ui::engine_ui);
#endif

	if (!engine.Init(sf::ContextSettings()))
		return EXIT_FAILURE;
	
	ImGui::SetCurrentContext(engine.GetImGuiContext());
	
	LOGDEBUG("starting main loop");
	engine.Run();

	return EXIT_SUCCESS;
}