#pragma once

#include "Script.h"

namespace Toad
{

class Movement : Script
{
	//SCRIPTBASE(Movement)
public:
	void OnUpdate() override;
};

}