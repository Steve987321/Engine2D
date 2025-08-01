#include "pch.h"
#include <EngineCore.h>
#include <engine/Engine.h>

#ifdef TOAD_EDITOR
#include "ui/UI.h"
#endif 

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char** argv)
#endif 
{	
#ifdef TOAD_EDITOR
	Toad::SetPreUICallback(ui::update_ini);
	Toad::SetEngineUI(ui::engine_ui);
	Toad::SetEventCallback(ui::event_callback);
	Toad::SetEditorTextureDrawCallback(ui::editor_texture_draw_callback);
	Toad::SetOnCloseCallback(ui::save_ini_files);
#endif

	if (!Toad::Init())
		return 1;
	
	ImGui::SetCurrentContext(Toad::GetWindow().GetImGuiContext());
	
	Toad::Run();

	return 0;
}
