#pragma once
class TestScript : public Toad::Script
{
public:
	SCRIPT_CONSTRUCT(TestScript)

	void OnUpdate(Toad::Object* obj) override;

};

