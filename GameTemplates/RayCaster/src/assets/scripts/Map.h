#pragma once

class MapObject;

// Script that holds the objects that have been found in the scene
class GAME_API Map : public Toad::Script
{
public:
	SCRIPT_CONSTRUCT(Map)

public:
	std::vector<MapObject*> objects;

public:
	void OnStart(Toad::Object* obj) override;
	void OnUpdate(Toad::Object* obj) override;
//
//#if defined(TOAD_EDITOR) || !defined(NDEBUG)
//	virtual void OnImGui(Toad::Object* obj, ImGuiContext* ctx);
//#endif 

	void ExposeVars() override;
};

