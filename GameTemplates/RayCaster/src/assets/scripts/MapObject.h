#pragma once

// should only be attched to sprites 
class GAME_API MapObject : public Toad::Script
{
public:
	SCRIPT_CONSTRUCT(MapObject)

	void OnStart(Toad::Object* obj) override;
	void OnUpdate(Toad::Object* obj) override;
	void ExposeVars() override;

	Toad::Sprite* spr = nullptr;

	bool active = true;
};
