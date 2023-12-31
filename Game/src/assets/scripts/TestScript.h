#pragma once

class GAME_API TestScript : public Toad::Script
{
public:
	SCRIPT_CONSTRUCT(TestScript)

	float start_direction_X = 1;
	float start_direction_Y = 1;
	float speed_mult = 1;

	float velx = 1;
	float vely = 1;

	void OnStart(Toad::Object* obj) override;
	void OnUpdate(Toad::Object* obj) override;
	void ExposeVars() override;
};
