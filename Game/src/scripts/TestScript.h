#pragma once

class GAME_API TestScript : public Toad::Script
{
public:
	SCRIPT_CONSTRUCT(TestScript)

	EXPOSABLE bool variable1 = false;
	EXPOSABLE float start_direction_X = 1;
	EXPOSABLE float start_direction_Y = 1;
	EXPOSABLE float speed_mult = 1;

	float velx = 1;
	float vely = 1;

	void OnStart(Toad::Object* obj) override;
	void OnUpdate(Toad::Object* obj) override;
	void ExposeVars() override;

	std::shared_ptr<Toad::Script> Clone() override
	{
		auto pScript = std::make_shared<TestScript>(*this);
		return std::shared_ptr<Toad::Script>(dynamic_cast<Toad::Script*>(pScript.get()));
	}

};

