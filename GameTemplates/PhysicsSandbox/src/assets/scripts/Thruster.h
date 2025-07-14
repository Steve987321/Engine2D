#pragma once

class GAME_API Thruster : public Toad::Script
{
public:
	SCRIPT_CONSTRUCT(Thruster)

	void OnStart(Toad::Object* obj) override;
	void OnUpdate(Toad::Object* obj) override;
	void ExposeVars() override;

public:
	float thrust = 1000.f;
	float angle = 0.f; 
	float mass = 10.f; 

	Toad::Vec2f velocity{0, 0};

	std::string attached_obj_name;
	Toad::Object* attached_obj = nullptr;
};

