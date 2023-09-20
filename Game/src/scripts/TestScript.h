#pragma once

class GAME_API TestScript : public Toad::Script
{
public:
	SCRIPT_CONSTRUCT(TestScript)

	EXPOSABLE bool variable1 = false;
	EXPOSABLE float start_direction_X = 1;
	EXPOSABLE float start_direction_Y = 1;
	EXPOSABLE float speed_mult = 1;

	void OnCreate(Toad::Object* obj) override;
	void OnUpdate(Toad::Object* obj) override;
	void ExposeVars() override;
};

