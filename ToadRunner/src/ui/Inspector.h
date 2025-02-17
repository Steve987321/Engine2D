#pragma once

namespace Toad
{
	class Object;
}

namespace ui
{ 
	class GameAssetsBrowser;

	// full aids 
	void object_inspector(Toad::Object*& selected_obj, const ui::GameAssetsBrowser& asset_browser);
}
