#pragma once

class GAME_API TestScript : public Toad::Script
{
public:
	SCRIPT_CONSTRUCT(TestScript)

	EXPOSABLE bool variable1 = false;

	void OnUpdate(Toad::Object* obj) override;
	void ExposeVars() override;
};

