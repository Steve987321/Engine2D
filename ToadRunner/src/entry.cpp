#include "pch.h"
#include <EngineCore.h>
#include <engine/Engine.h>

#include "ui/ui.h"

int main(int argc, char** argv)
{
	Toad::Engine engine;
	
#ifdef TOAD_EDITOR
	engine.SetEngineUI(ui::engine_ui);
	engine.SetEventCallback(ui::event_callback);
	engine.SetEditorTextureDrawCallback(ui::editor_texture_draw_callback);
#endif

	if (!engine.Init())
		return EXIT_FAILURE;
	
	ImGui::SetCurrentContext(engine.GetImGuiContext());
	
	engine.Run();

	return EXIT_SUCCESS;
}