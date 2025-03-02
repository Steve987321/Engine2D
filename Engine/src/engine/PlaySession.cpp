#include "pch.h"
#include "PlaySession.h"

#include "engine/systems/Scene.h"
#include "engine/DrawingCanvas.h"

namespace Toad
{
void StartGameSession()
{
	if (begin_play)
		return;

	begin_play = true;
	Scene::current_scene.Start();
}

void StopGameSession()
{
	if (begin_play)
		Scene::current_scene.End(nullptr);

	DrawingCanvas::Clear();

	begin_play = false;
}

}